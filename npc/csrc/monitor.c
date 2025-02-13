#include "common.h"
#include "pmem.h"
#include "getopt.h"
#include "sdb.h"
#include "utils.h"
#include "debug.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "remote_bitbang.h"
#include VTOP_H
#include "regs.h"

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *ELF_FILE = NULL;
static char *wave_file = NULL;
static int difftest_port = 0;

extern void init_log(const char *log_file);
extern void init_disasm(const char *triple);
extern void init_itrace();
extern void sim_rst();
extern void init_ftrace(const char *ELF_FILE);
extern void init_difftest(char *ref_so_file, long img_size, int port);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NPC!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}

static long load_img(){
    if (img_file == NULL){
        Log("No image is given. Use the default build-in image.");
        // exit(1);
        return 4096; // built-in image size
    }

    FILE *fp = fopen(img_file, "rb");
    // Assert(fp, "Can not open '%s'", img_file);
    if(fp==NULL){
        Log("No image is given. Use the default build-in image.");
        return 4096; // built-in image size
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    Log("The image is %s, size = %ld", img_file, size);

    fseek(fp, 0, SEEK_SET);
    int ret = fread(guest_to_host(PC_RST), size, 1, fp);
    assert(ret == 1);
    Log("successful read img");

    fclose(fp);
    return size;
}

static void parse_args(int argc,char *argv[]){
    // printf("Enter the search commnd mode\n");
    const struct option table[] = {
        {"batch",   no_argument,        NULL,   'b' },
        {"log",     required_argument,  NULL,   'l' },
        {"diff",    required_argument,  NULL,   'd' },
        {"port",    required_argument,  NULL,   'p' },
        {"wave",    required_argument,  NULL,   'w' },
        {"elf",     required_argument,  NULL,   'e' },
        {"help",    no_argument,        NULL,   'h' },
        {0,         0,                  NULL,   0   },
    };
    int o;
    while ((o = getopt_long(argc, argv, "-bhl:d:p:e:w:", table, NULL)) != -1)
    {
        switch (o)
        {
        case 'b':
            sdb_set_batch_mode();
            break;
        case 'p':
            sscanf(optarg, "%d", &difftest_port);
            break;
        case 'l':
            log_file = optarg;
            printf(ANSI_FMT("%s is log file\n", ANSI_FG_GREEN),log_file);
            break;
        case 'd':
            diff_so_file = optarg;
            break;
        case 'e':
            ELF_FILE = optarg;
            break;
        case 'w':
            wave_file = optarg;
            break;
        case 1:
            img_file = optarg;
            return;
        default:
            printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
            printf("\t-b,--batch              run with batch mode\n");
            printf("\t-l,--log=FILE           output log to FILE\n");
            printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
            printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
            printf("\t-e,--elf=ELF_FILE       open the elf file to search the symbol table\n");
            printf("\n");
            exit(0);
        }
  }
  return;
}

void init_monitor(VTOP *top, VerilatedFstC *tfp, remote_bitbang_t **remote_bitbang, int argc, char *argv[]){
    IFDEF(CONFIG_ITRACE, init_itrace());
    parse_args(argc, argv);
    init_log(log_file);
    IFDEF(CONFIG_FTRACE, init_ftrace(ELF_FILE));
    long img_size=load_img();
    init_sdb();
    Log("successful init sdb");
    init_disasm(MUXDEF(CONFIG_RV64, "riscv64", "riscv32") "-pc-linux-gnu");
    Log("successful init disasm");
    IFDEF(CONFIG_VCD_GET, tfp->open(wave_file));
    // Log(ANSI_FMT("Hello\n", ANSI_FG_GREEN));
    sim_rst();
    Log("successful sim rst\n");
    // isa_reg_display();
    *remote_bitbang = new remote_bitbang_t(difftest_port, top);
    init_difftest(diff_so_file, img_size, difftest_port);
    Log("successful init difftest\n");
    welcome();
    return;
}
