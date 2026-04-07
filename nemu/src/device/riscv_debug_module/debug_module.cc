#include <cassert>

#include <device/riscv_debug_module/debug_module.h>
#include <device/riscv_debug_module/jtag_dtm.h>
#include <device/riscv_debug_module/remote_bitbang.h>
#include <device/riscv_debug_module/debug_defines.h>
#include <device/riscv_debug_module/riscv_opcodes.h>
#include <device/riscv_debug_module/riscv_decode.h>
#include <device/riscv_debug_module/riscv_trap.h>

#include <device/riscv_debug_module/debug_rom.h>
#include <device/riscv_debug_module/debug_rom_defines.h>
extern "C"{
    #include "memory/paddr.h"
    #include <device/map.h>
    void riscv_cpu_restart();
    void riscv_cpu_halt();
}

#if 0
#define D(x) x
#else
#define D(x)
#endif

// Return the number of bits wide that a field has to be to encode up to n
// different values.
// 1->0, 2->1, 3->2, 4->2
static unsigned field_width(unsigned n)
{
    unsigned i = 0;
    n -= 1;
    while (n)
    {
        i++;
        n >>= 1;
    }
    return i;
}

///////////////////////// debug_module_t

debug_module_t::debug_module_t(const debug_module_config_t &config) : config(config),
                                                                      program_buffer_bytes((config.support_impebreak ? 4 : 0) + 4 * config.progbufsize),
                                                                      debug_progbuf_start(debug_data_start - program_buffer_bytes),
                                                                      debug_abstract_start(debug_progbuf_start - debug_abstract_size * 4),
                                                                      custom_base(0),
                                                                      // The spec lets a debugger select nonexistent harts. Create hart_state for
                                                                      // them because I'm too lazy to add the code to just ignore accesses.
                                                                      hart_state(1 << field_width(1)),
                                                                      rti_remaining(0)
{
    D(fprintf(stderr, "debug_data_start=0x%x\n", debug_data_start));
    D(fprintf(stderr, "debug_progbuf_start=0x%x\n", debug_progbuf_start));
    D(fprintf(stderr, "debug_abstract_start=0x%x\n", debug_abstract_start));

    const unsigned max_procs = 1024;
    if (0 >= max_procs)
    {
        fprintf(stderr, "Hart IDs must not exceed %u (%u harts with max hart ID %u requested)\n",
                max_procs - 1, 1, 0);
        exit(1);
    }

    program_buffer = new uint8_t[program_buffer_bytes];

    memset(debug_rom_flags, 0, sizeof(debug_rom_flags));
    memset(program_buffer, 0, program_buffer_bytes);
    memset(dmdata, 0, sizeof(dmdata));

    if (config.support_impebreak)
    {
        program_buffer[4 * config.progbufsize] = ebreak();
        program_buffer[4 * config.progbufsize + 1] = ebreak() >> 8;
        program_buffer[4 * config.progbufsize + 2] = ebreak() >> 16;
        program_buffer[4 * config.progbufsize + 3] = ebreak() >> 24;
    }

    write32(debug_rom_whereto, 0,
            jal(ZERO, debug_abstract_start - DEBUG_ROM_WHERETO));

    memset(debug_abstract, 0, sizeof(debug_abstract));

    reset();
}

debug_module_t::~debug_module_t()
{
    delete[] program_buffer;
}

void debug_module_t::reset()
{
    memset(&dmcontrol, 0, sizeof(dmcontrol));

    memset(&dmstatus, 0, sizeof(dmstatus));
    dmstatus.impebreak = config.support_impebreak;
    dmstatus.authenticated = !config.require_authentication;
    dmstatus.version = 2;

    memset(&abstractcs, 0, sizeof(abstractcs));
    abstractcs.datacount = sizeof(dmdata) / 4;
    abstractcs.progbufsize = config.progbufsize;

    memset(&abstractauto, 0, sizeof(abstractauto));

    memset(&sbcs, 0, sizeof(sbcs));
    if (config.max_sba_data_width > 0)
    {
        sbcs.version = 1;
        sbcs.asize = sizeof(reg_t) * 8;
    }
    if (config.max_sba_data_width >= 64)
        sbcs.access64 = true;
    if (config.max_sba_data_width >= 32)
        sbcs.access32 = true;
    if (config.max_sba_data_width >= 16)
        sbcs.access16 = true;
    if (config.max_sba_data_width >= 8)
        sbcs.access8 = true;

    challenge = random();
}

bool debug_module_t::load(reg_t addr, size_t len, uint8_t *bytes)
{
    addr = DEBUG_START + addr;

    if (addr >= DEBUG_ROM_ENTRY &&
        (addr + len) <= (DEBUG_ROM_ENTRY + debug_rom_raw_len))
    {
        memcpy(bytes, debug_rom_raw + addr - DEBUG_ROM_ENTRY, len);
        return true;
    }

    if (addr >= DEBUG_ROM_WHERETO && (addr + len) <= (DEBUG_ROM_WHERETO + 4))
    {
        memcpy(bytes, debug_rom_whereto + addr - DEBUG_ROM_WHERETO, len);
        return true;
    }

    if (addr >= DEBUG_ROM_FLAGS && ((addr + len) <= DEBUG_ROM_FLAGS + 1024))
    {
        memcpy(bytes, debug_rom_flags + addr - DEBUG_ROM_FLAGS, len);
        return true;
    }

    if (addr >= debug_abstract_start && ((addr + len) <= (debug_abstract_start + sizeof(debug_abstract))))
    {
        memcpy(bytes, debug_abstract + addr - debug_abstract_start, len);
        return true;
    }

    if (addr >= debug_data_start && (addr + len) <= (debug_data_start + sizeof(dmdata)))
    {
        memcpy(bytes, dmdata + addr - debug_data_start, len);
        return true;
    }

    if (addr >= debug_progbuf_start && ((addr + len) <= (debug_progbuf_start + program_buffer_bytes)))
    {
        memcpy(bytes, program_buffer + addr - debug_progbuf_start, len);
        return true;
    }

    D(fprintf(stderr, "ERROR: invalid load from debug module: %zd bytes at 0x%016" PRIx64 "\n", len, addr));

    return false;
}

bool debug_module_t::store(reg_t addr, size_t len, const uint8_t *bytes)
{
    D(
        switch (len) {
            case 4:
                fprintf(stderr, "store(addr=0x%lx, len=%d, bytes=0x%08x); "
                                "hartsel=0x%x\n",
                        addr, (unsigned)len, *(uint32_t *)bytes,
                        dmcontrol.hartsel);
                break;
            default:
                fprintf(stderr, "store(addr=0x%lx, len=%d, bytes=...); "
                                "hartsel=0x%x\n",
                        addr, (unsigned)len, dmcontrol.hartsel);
                break;
        });

    uint8_t id_bytes[4];
    uint32_t id = 0;
    if (len == 4)
    {
        memcpy(id_bytes, bytes, 4);
        id = read32(id_bytes, 0);
    }

    addr = DEBUG_START + addr;

    if (addr >= debug_data_start && (addr + len) <= (debug_data_start + sizeof(dmdata)))
    {
        memcpy(dmdata + addr - debug_data_start, bytes, len);
        return true;
    }

    if (addr >= debug_progbuf_start && ((addr + len) <= (debug_progbuf_start + program_buffer_bytes)))
    {
        memcpy(program_buffer + addr - debug_progbuf_start, bytes, len);

        return true;
    }

    if (addr == DEBUG_ROM_HALTED)
    {
        assert(len == 4);
        if (!hart_state[id].halted)
        {
            hart_state[id].halted = true;
        }
        if (selected_hart_id() == id)
        {
            if (0 == (debug_rom_flags[id] & (1 << DEBUG_ROM_FLAG_GO)))
            {
                abstract_command_completed = true;
            }
        }
        return true;
    }

    if (addr == DEBUG_ROM_GOING)
    {
        assert(len == 4);
        debug_rom_flags[id] &= ~(1 << DEBUG_ROM_FLAG_GO);
        return true;
    }

    if (addr == DEBUG_ROM_RESUMING)
    {
        assert(len == 4);
        hart_state[id].halted = false;
        hart_state[id].resumeack = true;
        debug_rom_flags[id] &= ~(1 << DEBUG_ROM_FLAG_RESUME);
        return true;
    }

    if (addr == DEBUG_ROM_EXCEPTION)
    {
        if (abstractcs.cmderr == CMDERR_NONE)
        {
            abstractcs.cmderr = CMDERR_EXCEPTION;
        }
        return true;
    }

    D(fprintf(stderr, "ERROR: invalid store to debug module: %zd bytes at 0x%016" PRIx64 "\n", len, addr));
    return false;
}

void debug_module_t::write32(uint8_t *memory, unsigned int index, uint32_t value)
{
    uint8_t *base = memory + index * 4;
    base[0] = value & 0xff;
    base[1] = (value >> 8) & 0xff;
    base[2] = (value >> 16) & 0xff;
    base[3] = (value >> 24) & 0xff;
}

uint32_t debug_module_t::read32(uint8_t *memory, unsigned int index)
{
    uint8_t *base = memory + index * 4;
    uint32_t value = ((uint32_t)base[0]) |
                     (((uint32_t)base[1]) << 8) |
                     (((uint32_t)base[2]) << 16) |
                     (((uint32_t)base[3]) << 24);
    return value;
}

bool debug_module_t::hart_selected(unsigned hartid) const
{
    return hartid == selected_hart_id();
}

unsigned debug_module_t::sb_access_bits()
{
    return 8 << sbcs.sbaccess;
}

void debug_module_t::sb_autoincrement()
{
    if (!sbcs.autoincrement || !config.max_sba_data_width)
        return;

    uint64_t value = sbaddress[0] + sb_access_bits() / 8;
    sbaddress[0] = value;
    uint32_t carry = value >> 32;

    value = sbaddress[1] + carry;
    sbaddress[1] = value;
    carry = value >> 32;

    value = sbaddress[2] + carry;
    sbaddress[2] = value;
    carry = value >> 32;

    sbaddress[3] += carry;
}

void debug_module_t::sb_read()
{
    reg_t address = ((uint64_t)sbaddress[1] << 32) | sbaddress[0];
    try
    {
        if (sbcs.sbaccess == 0 && config.max_sba_data_width >= 8)
        {
            sbdata[0] = paddr_read(address, 1);
        }
        else if (sbcs.sbaccess == 1 && config.max_sba_data_width >= 16)
        {
            sbdata[0] = paddr_read(address, 2);
        }
        else if (sbcs.sbaccess == 2 && config.max_sba_data_width >= 32)
        {
            sbdata[0] = paddr_read(address, 4);
        }
        else if (sbcs.sbaccess == 3 && config.max_sba_data_width >= 64)
        {
            uint64_t value = paddr_read(address, 8);
            sbdata[0] = value;
            sbdata[1] = value >> 32;
        }
        else
        {
            sbcs.error = 3;
        }
    }
    catch (trap_load_access_fault &t)
    {
        sbcs.error = 2;
    }
}

void debug_module_t::sb_write()
{
    reg_t address = ((uint64_t)sbaddress[1] << 32) | sbaddress[0];
    D(fprintf(stderr, "sb_write() 0x%x @ 0x%lx\n", sbdata[0], address));
    if (sbcs.sbaccess == 0 && config.max_sba_data_width >= 8)
    {
        paddr_write(address, 1, sbdata[0]);
    }
    else if (sbcs.sbaccess == 1 && config.max_sba_data_width >= 16)
    {
        paddr_write(address, 2, sbdata[0]);
    }
    else if (sbcs.sbaccess == 2 && config.max_sba_data_width >= 32)
    {
        paddr_write(address, 4, sbdata[0]);
    }
    else if (sbcs.sbaccess == 3 && config.max_sba_data_width >= 64)
    {
        paddr_write(address, 8, (((uint64_t)sbdata[1]) << 32) | sbdata[0]);
    }
    else
    {
        sbcs.error = 3;
    }
}

bool debug_module_t::dmi_read(unsigned address, uint32_t *value)
{
    uint32_t result = 0;
    D(fprintf(stderr, "dmi_read(0x%x) -> ", address));
    if (address >= DM_DATA0 && address < DM_DATA0 + abstractcs.datacount)
    {
        unsigned i = address - DM_DATA0;
        result = read32(dmdata, i);
        if (abstractcs.busy)
        {
            result = -1;
            D(fprintf(stderr, "\ndmi_read(0x%02x (data[%d]) -> -1 because abstractcs.busy==true\n", address, i));
        }

        if (abstractcs.busy && abstractcs.cmderr == CMDERR_NONE)
        {
            abstractcs.cmderr = CMDERR_BUSY;
        }

        if (!abstractcs.busy && ((abstractauto.autoexecdata >> i) & 1))
        {
            perform_abstract_command();
        }
    }
    else if (address >= DM_PROGBUF0 && address < DM_PROGBUF0 + config.progbufsize)
    {
        unsigned i = address - DM_PROGBUF0;
        result = read32(program_buffer, i);
        if (abstractcs.busy)
        {
            result = -1;
            D(fprintf(stderr, "\ndmi_read(0x%02x (progbuf[%d]) -> -1 because abstractcs.busy==true\n", address, i));
        }
        if (!abstractcs.busy && ((abstractauto.autoexecprogbuf >> i) & 1))
        {
            perform_abstract_command();
        }
    }
    else
    {
        switch (address)
        {
        case DM_DMCONTROL:
        {
            result = set_field(result, DM_DMCONTROL_HALTREQ, dmcontrol.haltreq);
            result = set_field(result, DM_DMCONTROL_RESUMEREQ, dmcontrol.resumereq);
            result = set_field(result, DM_DMCONTROL_HARTSELHI,
                               dmcontrol.hartsel >> DM_DMCONTROL_HARTSELLO_LENGTH);
            result = set_field(result, DM_DMCONTROL_HASEL, dmcontrol.hasel);
            result = set_field(result, DM_DMCONTROL_HARTSELLO, dmcontrol.hartsel);
            result = set_field(result, DM_DMCONTROL_HARTRESET, dmcontrol.hartreset);
            result = set_field(result, DM_DMCONTROL_NDMRESET, dmcontrol.ndmreset);
            result = set_field(result, DM_DMCONTROL_DMACTIVE, dmcontrol.dmactive);
        }
        break;
        case DM_DMSTATUS:
        {
            dmstatus.allhalted = true;
            dmstatus.anyhalted = false;
            dmstatus.allrunning = true;
            dmstatus.anyrunning = false;
            dmstatus.allresumeack = true;
            dmstatus.anyresumeack = false;
            dmstatus.allnonexistant = false;
            if (hart_state[0].resumeack)
            {
                dmstatus.anyresumeack = true;
            }
            else
            {
                dmstatus.allresumeack = false;
            }
            if (hart_state[0].halted)
            {
                dmstatus.allrunning = false;
                dmstatus.anyhalted = true;
            }
            else
            {
                dmstatus.allhalted = false;
                dmstatus.anyrunning = true;
            }

            // We don't allow selecting non-existant harts through
            // hart_array_mask, so the only way it's possible is by writing a
            // non-existant hartsel.
            dmstatus.anynonexistant = dmcontrol.hartsel >= 1;

            dmstatus.allunavail = false;
            dmstatus.anyunavail = false;

            result = set_field(result, DM_DMSTATUS_IMPEBREAK,
                               dmstatus.impebreak);
            result = set_field(result, DM_DMSTATUS_ALLHAVERESET, selected_hart_state().havereset);
            result = set_field(result, DM_DMSTATUS_ANYHAVERESET, selected_hart_state().havereset);
            result = set_field(result, DM_DMSTATUS_ALLNONEXISTENT, dmstatus.allnonexistant);
            result = set_field(result, DM_DMSTATUS_ALLUNAVAIL, dmstatus.allunavail);
            result = set_field(result, DM_DMSTATUS_ALLRUNNING, dmstatus.allrunning);
            result = set_field(result, DM_DMSTATUS_ALLHALTED, dmstatus.allhalted);
            result = set_field(result, DM_DMSTATUS_ALLRESUMEACK, dmstatus.allresumeack);
            result = set_field(result, DM_DMSTATUS_ANYNONEXISTENT, dmstatus.anynonexistant);
            result = set_field(result, DM_DMSTATUS_ANYUNAVAIL, dmstatus.anyunavail);
            result = set_field(result, DM_DMSTATUS_ANYRUNNING, dmstatus.anyrunning);
            result = set_field(result, DM_DMSTATUS_ANYHALTED, dmstatus.anyhalted);
            result = set_field(result, DM_DMSTATUS_ANYRESUMEACK, dmstatus.anyresumeack);
            result = set_field(result, DM_DMSTATUS_AUTHENTICATED, dmstatus.authenticated);
            result = set_field(result, DM_DMSTATUS_AUTHBUSY, dmstatus.authbusy);
            result = set_field(result, DM_DMSTATUS_VERSION, dmstatus.version);
        }
        break;
        case DM_ABSTRACTCS:
            result = set_field(result, DM_ABSTRACTCS_CMDERR, abstractcs.cmderr);
            result = set_field(result, DM_ABSTRACTCS_BUSY, abstractcs.busy);
            result = set_field(result, DM_ABSTRACTCS_DATACOUNT, abstractcs.datacount);
            result = set_field(result, DM_ABSTRACTCS_PROGBUFSIZE,
                               abstractcs.progbufsize);
            break;
        case DM_ABSTRACTAUTO:
            result = set_field(result, DM_ABSTRACTAUTO_AUTOEXECPROGBUF, abstractauto.autoexecprogbuf);
            result = set_field(result, DM_ABSTRACTAUTO_AUTOEXECDATA, abstractauto.autoexecdata);
            break;
        case DM_COMMAND:
            result = 0;
            break;
        case DM_HARTINFO:
            result = set_field(result, DM_HARTINFO_NSCRATCH, 1);
            result = set_field(result, DM_HARTINFO_DATAACCESS, 1);
            result = set_field(result, DM_HARTINFO_DATASIZE, abstractcs.datacount);
            result = set_field(result, DM_HARTINFO_DATAADDR, debug_data_start);
            break;
        case DM_SBCS:
            result = set_field(result, DM_SBCS_SBVERSION, sbcs.version);
            result = set_field(result, DM_SBCS_SBREADONADDR, sbcs.readonaddr);
            result = set_field(result, DM_SBCS_SBACCESS, sbcs.sbaccess);
            result = set_field(result, DM_SBCS_SBAUTOINCREMENT, sbcs.autoincrement);
            result = set_field(result, DM_SBCS_SBREADONDATA, sbcs.readondata);
            result = set_field(result, DM_SBCS_SBERROR, sbcs.error);
            result = set_field(result, DM_SBCS_SBASIZE, sbcs.asize);
            result = set_field(result, DM_SBCS_SBACCESS128, sbcs.access128);
            result = set_field(result, DM_SBCS_SBACCESS64, sbcs.access64);
            result = set_field(result, DM_SBCS_SBACCESS32, sbcs.access32);
            result = set_field(result, DM_SBCS_SBACCESS16, sbcs.access16);
            result = set_field(result, DM_SBCS_SBACCESS8, sbcs.access8);
            break;
        case DM_SBADDRESS0:
            result = sbaddress[0];
            break;
        case DM_SBADDRESS1:
            result = sbaddress[1];
            break;
        case DM_SBADDRESS2:
            result = sbaddress[2];
            break;
        case DM_SBADDRESS3:
            result = sbaddress[3];
            break;
        case DM_SBDATA0:
            result = sbdata[0];
            if (sbcs.error == 0)
            {
                if (sbcs.readondata)
                {
                    sb_read();
                }
                if (sbcs.error == 0)
                {
                    sb_autoincrement();
                }
            }
            break;
        case DM_SBDATA1:
            result = sbdata[1];
            break;
        case DM_SBDATA2:
            result = sbdata[2];
            break;
        case DM_SBDATA3:
            result = sbdata[3];
            break;
        default:
            result = 0;
            D(fprintf(stderr, "Unexpected. Returning Error."));
            return false;
        }
    }
    D(fprintf(stderr, "0x%x\n", result));
    *value = result;
    return true;
}

void debug_module_t::run_test_idle()
{
    if (rti_remaining > 0)
    {
        rti_remaining--;
    }
    if (rti_remaining == 0 && abstractcs.busy && abstract_command_completed)
    {
        abstractcs.busy = false;
    }
}

static bool is_fpu_reg(unsigned regno)
{
    return (regno >= 0x1020 && regno <= 0x103f) || regno == CSR_FFLAGS ||
           regno == CSR_FRM || regno == CSR_FCSR;
}

bool debug_module_t::perform_abstract_command()
{
    if (abstractcs.cmderr != CMDERR_NONE)
        return true;
    if (abstractcs.busy)
    {
        abstractcs.cmderr = CMDERR_BUSY;
        return true;
    }

    if ((command >> 24) == 0)
    {
        // register access
        unsigned size = get_field(command, AC_ACCESS_REGISTER_AARSIZE);
        bool write = get_field(command, AC_ACCESS_REGISTER_WRITE);
        unsigned regno = get_field(command, AC_ACCESS_REGISTER_REGNO);

        if (!selected_hart_state().halted)
        {
            abstractcs.cmderr = CMDERR_HALTRESUME;
            return true;
        }

        unsigned i = 0;
        if (get_field(command, AC_ACCESS_REGISTER_TRANSFER))
        {

            if (is_fpu_reg(regno))
            {
                // Save S0
                write32(debug_abstract, i++, csrw(S0, CSR_DSCRATCH0));
                // Save mstatus
                write32(debug_abstract, i++, csrr(S0, CSR_MSTATUS));
                write32(debug_abstract, i++, csrw(S0, CSR_DSCRATCH1));
                // Set mstatus.fs
                assert((MSTATUS_FS & 0xfff) == 0);
                write32(debug_abstract, i++, lui(S0, MSTATUS_FS >> 12));
                write32(debug_abstract, i++, csrrs(ZERO, S0, CSR_MSTATUS));
            }

            if (regno < 0x1000 && config.support_abstract_csr_access)
            {
                if (!is_fpu_reg(regno))
                {
                    write32(debug_abstract, i++, csrw(S0, CSR_DSCRATCH0));
                }

                if (write)
                {
                    switch (size)
                    {
                    case 2:
                        write32(debug_abstract, i++, lw(S0, ZERO, debug_data_start));
                        break;
                    case 3:
                        write32(debug_abstract, i++, ld(S0, ZERO, debug_data_start));
                        break;
                    default:
                        abstractcs.cmderr = CMDERR_NOTSUP;
                        return true;
                    }
                    write32(debug_abstract, i++, csrw(S0, regno));
                }
                else
                {
                    write32(debug_abstract, i++, csrr(S0, regno));
                    switch (size)
                    {
                    case 2:
                        write32(debug_abstract, i++, sw(S0, ZERO, debug_data_start));
                        break;
                    case 3:
                        write32(debug_abstract, i++, sd(S0, ZERO, debug_data_start));
                        break;
                    default:
                        abstractcs.cmderr = CMDERR_NOTSUP;
                        return true;
                    }
                }
                if (!is_fpu_reg(regno))
                {
                    write32(debug_abstract, i++, csrr(S0, CSR_DSCRATCH0));
                }
            }
            else if (regno >= 0x1000 && regno < 0x1020)
            {
                unsigned regnum = regno - 0x1000;

                switch (size)
                {
                case 2:
                    if (write)
                        write32(debug_abstract, i++, lw(regnum, ZERO, debug_data_start));
                    else
                        write32(debug_abstract, i++, sw(regnum, ZERO, debug_data_start));
                    break;
                case 3:
                    if (write)
                        write32(debug_abstract, i++, ld(regnum, ZERO, debug_data_start));
                    else
                        write32(debug_abstract, i++, sd(regnum, ZERO, debug_data_start));
                    break;
                default:
                    abstractcs.cmderr = CMDERR_NOTSUP;
                    return true;
                }

                if (regno == 0x1000 + S0 && write)
                {
                    /*
                     * The exception handler starts out be restoring dscratch to s0,
                     * which was saved before executing the abstract memory region. Since
                     * we just wrote s0, also make sure to write that same value to
                     * dscratch in case an exception occurs in a program buffer that
                     * might be executed later.
                     */
                    write32(debug_abstract, i++, csrw(S0, CSR_DSCRATCH0));
                }
            }
            else if (regno >= 0x1020 && regno < 0x1040 && config.support_abstract_fpr_access)
            {
                unsigned fprnum = regno - 0x1020;

                if (write)
                {
                    switch (size)
                    {
                    case 2:
                        write32(debug_abstract, i++, flw(fprnum, ZERO, debug_data_start));
                        break;
                    case 3:
                        write32(debug_abstract, i++, fld(fprnum, ZERO, debug_data_start));
                        break;
                    default:
                        abstractcs.cmderr = CMDERR_NOTSUP;
                        return true;
                    }
                }
                else
                {
                    switch (size)
                    {
                    case 2:
                        write32(debug_abstract, i++, fsw(fprnum, ZERO, debug_data_start));
                        break;
                    case 3:
                        write32(debug_abstract, i++, fsd(fprnum, ZERO, debug_data_start));
                        break;
                    default:
                        abstractcs.cmderr = CMDERR_NOTSUP;
                        return true;
                    }
                }
            }
            else if (regno >= 0xc000 && (regno & 1) == 1)
            {
                // Support odd-numbered custom registers, to allow for debugger testing.
                unsigned custom_number = regno - 0xc000;
                abstractcs.cmderr = CMDERR_NONE;
                if (write)
                {
                    // Writing V to custom register N will cause future reads of N to
                    // return V, reads of N-1 will return V-1, etc.
                    custom_base = read32(dmdata, 0) - custom_number;
                }
                else
                {
                    write32(dmdata, 0, custom_number + custom_base);
                    write32(dmdata, 1, 0);
                }
                return true;
            }
            else
            {
                abstractcs.cmderr = CMDERR_NOTSUP;
                return true;
            }

            if (is_fpu_reg(regno))
            {
                // restore mstatus
                write32(debug_abstract, i++, csrr(S0, CSR_DSCRATCH1));
                write32(debug_abstract, i++, csrw(S0, CSR_MSTATUS));
                // restore s0
                write32(debug_abstract, i++, csrr(S0, CSR_DSCRATCH0));
            }
        }

        if (get_field(command, AC_ACCESS_REGISTER_POSTEXEC))
        {
            write32(debug_abstract, i,
                    jal(ZERO, debug_progbuf_start - debug_abstract_start - 4 * i));
            i++;
        }
        else
        {
            write32(debug_abstract, i++, ebreak());
        }

        debug_rom_flags[selected_hart_id()] |= 1 << DEBUG_ROM_FLAG_GO;
        rti_remaining = config.abstract_rti;
        abstract_command_completed = false;

        abstractcs.busy = true;
    }
    else
    {
        abstractcs.cmderr = CMDERR_NOTSUP;
    }
    return true;
}

bool debug_module_t::dmi_write(unsigned address, uint32_t value)
{
    D(fprintf(stderr, "dmi_write(0x%x, 0x%x)\n", address, value));

    if (!dmstatus.authenticated && address != DM_AUTHDATA &&
        address != DM_DMCONTROL)
        return false;

    if (address >= DM_DATA0 && address < DM_DATA0 + abstractcs.datacount)
    {
        unsigned i = address - DM_DATA0;
        if (!abstractcs.busy)
            write32(dmdata, address - DM_DATA0, value);

        if (abstractcs.busy && abstractcs.cmderr == CMDERR_NONE)
        {
            abstractcs.cmderr = CMDERR_BUSY;
        }

        if (!abstractcs.busy && ((abstractauto.autoexecdata >> i) & 1))
        {
            perform_abstract_command();
        }
        return true;
    }
    else if (address >= DM_PROGBUF0 && address < DM_PROGBUF0 + config.progbufsize)
    {
        unsigned i = address - DM_PROGBUF0;

        if (!abstractcs.busy)
            write32(program_buffer, i, value);

        if (!abstractcs.busy && ((abstractauto.autoexecprogbuf >> i) & 1))
        {
            perform_abstract_command();
        }
        return true;
    }
    else
    {
        switch (address)
        {
        case DM_DMCONTROL:
        {
            if (!dmcontrol.dmactive && get_field(value, DM_DMCONTROL_DMACTIVE))
                reset();
            dmcontrol.dmactive = get_field(value, DM_DMCONTROL_DMACTIVE);
            if (!dmstatus.authenticated || !dmcontrol.dmactive)
                return true;

            dmcontrol.haltreq = get_field(value, DM_DMCONTROL_HALTREQ);
            dmcontrol.resumereq = get_field(value, DM_DMCONTROL_RESUMEREQ);
            dmcontrol.hartreset = get_field(value, DM_DMCONTROL_HARTRESET);
            dmcontrol.ndmreset = get_field(value, DM_DMCONTROL_NDMRESET);
            dmcontrol.hasel = 0;
            dmcontrol.hartsel = 0;
            if (get_field(value, DM_DMCONTROL_ACKHAVERESET))
            {
                hart_state[0].havereset = false;
            }
            if (dmcontrol.haltreq)
            {
                riscv_cpu_halt();
                D(fprintf(stderr, "halt hart %d\n", 0));
            }
            if (dmcontrol.resumereq)
            {
                D(fprintf(stderr, "resume hart %d\n", 0));
                debug_rom_flags[0] |= (1 << DEBUG_ROM_FLAG_RESUME);
                hart_state[0].resumeack = false;
            }
            if (dmcontrol.hartreset)
            {
                riscv_cpu_restart();
                proc_reset(0);
            }

            if (dmcontrol.ndmreset)
            {
                riscv_cpu_restart();
                proc_reset(0);
            }
        }
            return true;

        case DM_COMMAND:
            command = value;
            return perform_abstract_command();

        case DM_ABSTRACTCS:
            abstractcs.cmderr = (cmderr_t)(((uint32_t)(abstractcs.cmderr)) & (~(uint32_t)(get_field(value, DM_ABSTRACTCS_CMDERR))));
            return true;

        case DM_ABSTRACTAUTO:
            abstractauto.autoexecprogbuf = get_field(value,
                                                     DM_ABSTRACTAUTO_AUTOEXECPROGBUF);
            abstractauto.autoexecdata = get_field(value,
                                                  DM_ABSTRACTAUTO_AUTOEXECDATA);
            return true;
        case DM_SBCS:
            sbcs.readonaddr = get_field(value, DM_SBCS_SBREADONADDR);
            sbcs.sbaccess = get_field(value, DM_SBCS_SBACCESS);
            sbcs.autoincrement = get_field(value, DM_SBCS_SBAUTOINCREMENT);
            sbcs.readondata = get_field(value, DM_SBCS_SBREADONDATA);
            sbcs.error &= ~get_field(value, DM_SBCS_SBERROR);
            return true;
        case DM_SBADDRESS0:
            sbaddress[0] = value;
            if (sbcs.error == 0 && sbcs.readonaddr)
            {
                sb_read();
                sb_autoincrement();
            }
            return true;
        case DM_SBADDRESS1:
            sbaddress[1] = value;
            return true;
        case DM_SBADDRESS2:
            sbaddress[2] = value;
            return true;
        case DM_SBADDRESS3:
            sbaddress[3] = value;
            return true;
        case DM_SBDATA0:
            sbdata[0] = value;
            if (sbcs.error == 0)
            {
                sb_write();
                if (sbcs.error == 0)
                {
                    sb_autoincrement();
                }
            }
            return true;
        case DM_SBDATA1:
            sbdata[1] = value;
            return true;
        case DM_SBDATA2:
            sbdata[2] = value;
            return true;
        case DM_SBDATA3:
            sbdata[3] = value;
            return true;
        }
    }
    return false;
}

void debug_module_t::proc_reset(unsigned id)
{
    hart_state[id].havereset = true;
    hart_state[id].halted = false;
}

hart_debug_state_t &debug_module_t::selected_hart_state()
{
    return hart_state[selected_hart_id()];
}

size_t debug_module_t::selected_hart_id() const
{
    return 0;
}

static debug_module_t *debug_module = NULL;
static jtag_dtm_t *jtag_dtm = NULL;
static remote_bitbang_t *remote_bitbang = NULL;
static uint8_t *debug_module_base = NULL;
static debug_module_config_t difftest_dm_config = {
    .progbufsize = 16,
    .max_sba_data_width = 64,
    .require_authentication = false,
    .abstract_rti = 0,
    .support_hasel = false,
    .support_abstract_csr_access = true,
    .support_abstract_fpr_access = false,
    .support_haltgroups = false,
    .support_impebreak = false
};

static void riscv_debug_module_io_handler(uint32_t offset, int len, bool is_write){
    assert(len <= 8);
    if (is_write){
        debug_module->store(offset, len, (debug_module_base + offset));
    }else{
        debug_module->load(offset, len, (debug_module_base + offset));
    }
}
extern "C" {
    void riscv_debug_module_proc_reset(unsigned id){
        debug_module->proc_reset(id);
    }

    void riscv_dm_update(){
        remote_bitbang->tick();
    }

    void riscv_dm_step_check(){
        if(((cpu.dcsr >> 2) & 0x1) && (cpu.debug_mode == false)){
            cpu.setp_check = true;
        }
    }

    void init_riscv_debug_module(uint16_t port){
        debug_module = new debug_module_t(difftest_dm_config);
        jtag_dtm = new jtag_dtm_t(debug_module, 0);
        remote_bitbang = new remote_bitbang_t(port, jtag_dtm);
        debug_module_base = new_space(4 * 1024);
        add_mmio_map("riscv debug module", CONFIG_RISCV_DM_MMIO, debug_module_base, 4 * 1024, riscv_debug_module_io_handler);
    }
}