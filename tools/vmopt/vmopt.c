#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/dump.h"
#include "mruby/string.h"
#include "mruby/proc.h"

extern const char vmopt_irep[];

void
mrb_init_vmopt(mrb_state *mrb)
{
  int n = mrb_read_irep(mrb, vmopt_irep);

  extern mrb_value mrb_top_self(mrb_state *mrb);
  mrb_run(mrb, mrb_proc_new(mrb, mrb->irep[n]), mrb_top_self(mrb));
}
const char vmopt_irep[] = {
0x52,0x49,0x54,0x45,0x30,0x30,0x30,0x39,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x39,
0x30,0x30,0x30,0x30,0x4d,0x41,0x54,0x5a,0x20,0x20,0x20,0x20,0x30,0x30,0x30,0x39,
0x30,0x30,0x30,0x30,0x00,0x00,0x04,0x03,0x00,0x0b,0x00,0x00,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0xd8,0x3e,0x00,0x00,0x00,0x3f,0x53,0x43,0x00,0x02,0x00,0x03,
0x00,0x02,0x1f,0xcf,0x00,0x00,0x00,0x07,0x01,0x00,0x00,0x05,0x01,0x00,0x00,0x44,
0x01,0x00,0x00,0xc5,0x01,0x00,0x00,0x05,0x01,0x00,0x00,0x44,0x01,0x00,0x02,0x45,
0x00,0x00,0x00,0x4a,0xa7,0x5d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x00,0x05,0x56,0x6d,0x4f,0x70,0x74,0xa1,0xa2,0x00,0x00,0x00,0x5b,0x53,0x43,0x00,
0x02,0x00,0x04,0x00,0x02,0x6f,0x28,0x00,0x00,0x00,0x09,0x01,0x00,0x00,0x06,0x01,
0x80,0x00,0x91,0x02,0x00,0x00,0x05,0x01,0x00,0x00,0xa0,0x01,0x00,0x00,0x48,0x01,
0x80,0x02,0xc0,0x01,0x00,0x80,0x46,0x01,0x00,0x00,0x05,0x01,0x00,0x00,0x29,0x9f,
0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x07,0x69,0x6e,0x63,
0x6c,0x75,0x64,0x65,0x00,0x08,0x49,0x6e,0x73,0x74,0x55,0x74,0x69,0x6c,0x00,0x06,
0x64,0x69,0x73,0x61,0x73,0x6d,0x9d,0x4d,0x00,0x00,0x00,0x76,0x53,0x43,0x00,0x06,
0x00,0x08,0x00,0x02,0x72,0x64,0x00,0x00,0x00,0x0e,0x02,0x00,0x00,0x26,0x03,0x00,
0x00,0x11,0x03,0x80,0x40,0x01,0x04,0x00,0x00,0x05,0x03,0x00,0x40,0xa0,0x01,0x81,
0x80,0x01,0x03,0x00,0xc0,0x01,0x03,0x80,0x00,0x05,0x03,0x00,0x80,0x20,0x02,0x01,
0x80,0x01,0x03,0x01,0x00,0x01,0x03,0x80,0x03,0x40,0x03,0x00,0xc0,0x20,0x03,0x00,
0x00,0x29,0xb2,0x91,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x04,
0x49,0x72,0x65,0x70,0x00,0x0e,0x67,0x65,0x74,0x5f,0x69,0x72,0x65,0x70,0x5f,0x62,
0x79,0x5f,0x6e,0x6f,0x00,0x04,0x69,0x73,0x65,0x71,0x00,0x04,0x65,0x61,0x63,0x68,
0xb2,0x38,0x00,0x00,0x00,0x67,0x53,0x43,0x00,0x04,0x00,0x07,0x00,0x02,0xed,0xeb,
0x00,0x00,0x00,0x0d,0x02,0x00,0x00,0x26,0x02,0x00,0x00,0x06,0x02,0x80,0x00,0x06,
0x03,0x00,0x40,0x01,0x03,0x80,0x00,0x05,0x02,0x80,0x40,0xa0,0x03,0x00,0x00,0x05,
0x02,0x00,0x00,0xa0,0x02,0x00,0x00,0x06,0x02,0x80,0x00,0x3d,0x03,0x00,0x00,0x05,
0x02,0x00,0x00,0xa0,0x02,0x00,0x00,0x29,0xa0,0x77,0x00,0x00,0x00,0x01,0x0f,0x00,
0x01,0x0a,0x26,0x04,0x00,0x00,0x00,0x02,0x00,0x05,0x70,0x72,0x69,0x6e,0x74,0x00,
0x0a,0x67,0x65,0x74,0x5f,0x6f,0x70,0x63,0x6f,0x64,0x65,0x79,0x57,0x00,0x00,0x00,
0x36,0x53,0x43,0x00,0x02,0x00,0x03,0x00,0x02,0x1f,0xcf,0x00,0x00,0x00,0x04,0x01,
0x00,0x00,0x05,0x01,0x00,0x00,0x44,0x01,0x00,0x00,0xc5,0x01,0x80,0x00,0x29,0x30,
0x85,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x08,0x49,0x6e,0x73,
0x74,0x55,0x74,0x69,0x6c,0xf7,0x5c,0x00,0x00,0x00,0x95,0x53,0x43,0x00,0x02,0x00,
0x03,0x00,0x02,0x1f,0xcf,0x00,0x00,0x00,0x11,0x01,0x00,0x00,0x48,0x01,0x80,0x02,
0xc0,0x01,0x00,0x00,0x46,0x01,0x00,0x00,0x48,0x01,0x80,0x04,0xc0,0x01,0x00,0x40,
0x46,0x01,0x00,0x00,0x48,0x01,0x80,0x06,0xc0,0x01,0x00,0x80,0x46,0x01,0x00,0x00,
0x48,0x01,0x80,0x08,0xc0,0x01,0x00,0xc0,0x46,0x01,0x00,0x00,0x48,0x01,0x80,0x0a,
0xc0,0x01,0x01,0x00,0x46,0x01,0x00,0x00,0x05,0x01,0x00,0x00,0x29,0xa8,0xa1,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x0a,0x67,0x65,0x74,0x5f,0x6f,
0x70,0x63,0x6f,0x64,0x65,0x00,0x08,0x67,0x65,0x74,0x61,0x72,0x67,0x5f,0x41,0x00,
0x08,0x67,0x65,0x74,0x61,0x72,0x67,0x5f,0x42,0x00,0x09,0x67,0x65,0x74,0x61,0x72,
0x67,0x5f,0x42,0x78,0x00,0x08,0x67,0x65,0x74,0x61,0x72,0x67,0x5f,0x43,0x4c,0xee,
0x00,0x00,0x00,0x37,0x53,0x43,0x00,0x04,0x00,0x06,0x00,0x02,0xfd,0xca,0x00,0x00,
0x00,0x06,0x02,0x00,0x00,0x26,0x02,0x00,0x40,0x01,0x02,0xc0,0x3f,0x03,0x03,0x00,
0x00,0x05,0x02,0x00,0x00,0xa0,0x02,0x00,0x00,0x29,0x70,0x22,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x26,0x32,0x17,0x00,0x00,0x00,0x47,0x53,
0x43,0x00,0x04,0x00,0x06,0x00,0x02,0xfd,0xca,0x00,0x00,0x00,0x09,0x02,0x00,0x00,
0x26,0x02,0x00,0x40,0x01,0x02,0xc0,0x0b,0x03,0x03,0x00,0x00,0x05,0x02,0x00,0x00,
0xa0,0x02,0xc0,0xff,0x03,0x03,0x00,0x00,0x05,0x02,0x00,0x40,0xa0,0x02,0x00,0x00,
0x29,0x32,0x47,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x3e,
0x3e,0x00,0x01,0x26,0x9f,0x30,0x00,0x00,0x00,0x47,0x53,0x43,0x00,0x04,0x00,0x06,
0x00,0x02,0xfd,0xca,0x00,0x00,0x00,0x09,0x02,0x00,0x00,0x26,0x02,0x00,0x40,0x01,
0x02,0xc0,0x06,0x83,0x03,0x00,0x00,0x05,0x02,0x00,0x00,0xa0,0x02,0xc0,0xff,0x03,
0x03,0x00,0x00,0x05,0x02,0x00,0x40,0xa0,0x02,0x00,0x00,0x29,0xe2,0xe0,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x3e,0x3e,0x00,0x01,0x26,0x9f,
0x30,0x00,0x00,0x00,0x4f,0x53,0x43,0x00,0x04,0x00,0x06,0x00,0x02,0xfd,0xca,0x00,
0x00,0x00,0x09,0x02,0x00,0x00,0x26,0x02,0x00,0x40,0x01,0x02,0xc0,0x03,0x03,0x03,
0x00,0x00,0x05,0x02,0x00,0x00,0xa0,0x02,0x80,0x00,0x02,0x03,0x00,0x00,0x05,0x02,
0x00,0x40,0xa0,0x02,0x00,0x00,0x29,0x25,0x15,0x00,0x00,0x00,0x01,0x03,0x00,0x05,
0x36,0x35,0x35,0x33,0x35,0x12,0x62,0x00,0x00,0x00,0x02,0x00,0x02,0x3e,0x3e,0x00,
0x01,0x26,0x9f,0x30,0x00,0x00,0x00,0x47,0x53,0x43,0x00,0x04,0x00,0x06,0x00,0x02,
0xfd,0xca,0x00,0x00,0x00,0x09,0x02,0x00,0x00,0x26,0x02,0x00,0x40,0x01,0x02,0xc0,
0x03,0x03,0x03,0x00,0x00,0x05,0x02,0x00,0x00,0xa0,0x02,0xc0,0x3f,0x03,0x03,0x00,
0x00,0x05,0x02,0x00,0x40,0xa0,0x02,0x00,0x00,0x29,0xed,0x9f,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x3e,0x3e,0x00,0x01,0x26,0x9f,0x30,0x00,
0x00,0x00,0x00,
};
