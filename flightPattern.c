/*
 * flightPattern.c
 *
 *  Created on: Apr 26, 2020
 *      Author: jesse
 */

#include "flightPattern.h"
#include "flashPattern.h"
#include "Nosecone.h"
#include "regulator.h"
#include "settings.h"
#include <stddef.h>
#include <string.h>

#define MAX_SOLID_BRT 15
//max brightness for a color that is not brighter than pur R, G, or B
#define MAX_SINGLE_BRT 31

const FLIGHT_PATTERN
              flight_patterns[]
                              __attribute__((section(".far_const")))=
                              {
                                 {"red",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,
                                               {.basic={.pattern=LED_PAT_WAVE_BIG_D,.value=12,.color={.brt=7,.r=0xFF,.g=0x00,.b=0x00}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                           },
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,
                                              {.basic={.pattern=LED_PAT_BOOST,.value=14,.color={.brt=MAX_SOLID_BRT,.r=0xFF,.g=0,.b=0}}},
                                              {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedGraph,.data=&(LED_color){.brt=MAX_SOLID_BRT,.r=0xFF,.g=0,.b=0}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedGraph,.data=&(LED_color){.brt=MAX_SOLID_BRT,.r=0xFF,.g=0,.b=0}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightGraph,.data=&(LED_color){.brt=MAX_SOLID_BRT,.r=0xFF,.g=0,.b=0}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_FADE,.val1=600,.val2=NC_MAX_PWM,.t1=10,.t2=8}
                                          },
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightParticle,.data=&(LED_color){.brt=31,.r=0xFF,.g=0x00,.b=0x00}}},
                                               {.mode=NC_MODE_INVALID},{.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM}
                                          },
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_SATURATION,.color={.brt=2,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=150},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                         }
                                 },
                                 {"shRed",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,
                                               {.basic={.pattern=LED_PAT_WAVE_BIG_D,.value=12,.color={.brt=7,.r=0xFF,.g=0x00,.b=0x00}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                           },
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,
                                              {.basic={.pattern=LED_PAT_BOOST,.value=14,.color={.brt=MAX_SOLID_BRT,.r=0xFF,.g=0,.b=0}}},
                                              {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedHue,.data=&(LED_color){.brt=MAX_SOLID_BRT,.h=0,.s=255,.v=200}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedHue,.data=&(LED_color){.brt=MAX_SOLID_BRT,.h=0,.s=255,.v=200}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightParticle,.data=&(LED_color){.brt=31,.r=0xFF,.g=0x00,.b=0x00}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_FADE,.val1=600,.val2=NC_MAX_PWM,.t1=10,.t2=8}
                                          },
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightParticle,.data=&(LED_color){.brt=31,.r=0xFF,.g=0x00,.b=0x00}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM}
                                          },
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_SATURATION,.color={.brt=2,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=150},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                         }
                                 },
                                 {"green",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_WAVE_BIG_D,.value=12,.color={.brt=7,.r=0x00,.g=0xFF,.b=0x00}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_BOOST,.value=14,.color={.brt=MAX_SOLID_BRT,.r=0,.g=0xFF,.b=0}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedGraph,.data=&(LED_color){.brt=MAX_SOLID_BRT,.r=0,.g=0xFF,.b=0}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedGraph,.data=&(LED_color){.brt=MAX_SOLID_BRT,.r=0,.g=0xFF,.b=0}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightGraph,.data=&(LED_color){.brt=MAX_SOLID_BRT,.r=0,.g=0xFF,.b=0}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_FADE,.val1=600,.val2=NC_MAX_PWM,.t1=10,.t2=8}
                                          },
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightParticle,.data=&(LED_color){.brt=31,.r=0x00,.g=0xFF,.b=0x00}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM}
                                          },
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_SATURATION,.color={.brt=2,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                           {.mode=NC_MODE_STATIC,.val1=150},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                         }
                                 },
                                 {"shGreen",{
                                             //Startup pattern
                                             {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                                  {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                             },
                                             //Idle pattern
                                             {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                                  {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                             },
                                             //Pad pattern
                                             {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_WAVE_BIG_D,.value=12,.color={.brt=7,.r=0x00,.g=0xFF,.b=0x00}}},
                                                  {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                             },
                                             //Boost pattern
                                             {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_BOOST,.value=14,.color={.brt=MAX_SOLID_BRT,.r=0,.g=0xFF,.b=0}}},
                                                  {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                             },
                                             //Fast pattern
                                             {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedHue,.data=&(LED_color){.brt=MAX_SOLID_BRT,.h=85,.s=255,.v=200}}},
                                                  {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                             },
                                             //Coast pattern
                                             {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedHue,.data=&(LED_color){.brt=MAX_SOLID_BRT,.h=85,.s=255,.v=200}}},
                                                  {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                             },
                                             //Drogue pattern
                                             {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightGraph,.data=&(LED_color){.brt=MAX_SOLID_BRT,.r=0,.g=0xFF,.b=0}}},
                                                  {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_FADE,.val1=600,.val2=NC_MAX_PWM,.t1=10,.t2=8}
                                             },
                                             //Main pattern
                                             {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightParticle,.data=&(LED_color){.brt=31,.r=0x00,.g=0xFF,.b=0x00}}},
                                                  {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM}
                                             },
                                             //Landed pattern
                                             {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_SATURATION,.color={.brt=2,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                              {.mode=NC_MODE_STATIC,.val1=150},{.mode=NC_MODE_STATIC,.val1=0}
                                             },
                                             //Invalid pattern
                                             {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                                  {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                             },
                                             //Test pattern
                                             {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                                  {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                             },
                                            }
                                 },
                                 {"USA",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_LIST_UNIFORM_PARTICLE,.value=5,.color={.brt=7,.r=0x00,.g=0x00,.b=0xFF},.list=&USA_colors}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&USA_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&USA_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&USA_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_STR_ST,.color={.brt=MAX_SINGLE_BRT,.r=0xFF,.g=0xFF,.b=0xFF},.list=&USA_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_FADE,.val1=600,.val2=NC_MAX_PWM,.t1=10,.t2=8}
                                          },
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightListParticle,.data=(void*)&USA_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM}
                                          },
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_FLOW_LIST,.value=2,.color={.brt=4,.r=0x00,.g=0x00,.b=0xFF},.list=&USA_RW_colors}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                         }
                                 },
                                 {"USAspk",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_LIST_UNIFORM_PARTICLE,.value=6,.color={.brt=5,.r=0x00,.g=0x00,.b=0xFF},.list=&USA_colors}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&USA_spk_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&USA_spk_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&USA_spk_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_STR_ST,.color={.brt=MAX_SINGLE_BRT,.r=0xFF,.g=0xFF,.b=0xFF},.list=&USA_spk_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_FADE,.val1=600,.val2=NC_MAX_PWM,.t1=10,.t2=8}
                                          },
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightListParticle,.data=(void*)&USA_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM}
                                          },
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_FLOW_LIST,.value=2,.color={.brt=4,.r=0x00,.g=0x00,.b=0xFF},.list=&USA_RW_colors}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                         }
                                 },
                                 {"rnbw",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_LIST_UNIFORM_PARTICLE,.value=6,.color={.brt=5,.r=255,.g=255,.b=255},.list=&RNBW_colors}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&RNBW_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&RNBW_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_colorHeight,.data=(void*)&RNBW_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_ST_LIST,.value=8,.color={.brt=MAX_SINGLE_BRT,.r=155,.g=100,.b=50},.list=&RNBW_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_ONE_SHOT,.val1=0,.val2=NC_MAX_PWM,.t1=8000}
                                          },
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_heightListParticle,.data=(void*)&RNBW_colors}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM}
                                          },
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_FLOW_LIST,.value=9,.color={.brt=4,.r=255,.g=200,.b=150},.list=&RNBW_colors}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                         }
                                 },
                                 {"sparks",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_PARTICLE,.value=4,{.brt=5,.r=255,.g=150,.b=10}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,
                                               {.basic={.pattern=LED_PAT_BOOST,.value=12,.color={.brt=MAX_SOLID_BRT-5,.r=255,.g=150,.b=10}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,
                                               {.basic={.pattern=LED_SOLID_ST,.value=0,.color={.brt=MAX_SOLID_BRT-6,.r=255,.g=150,.b=10}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,
                                               {.basic={.pattern=LED_SOLID_ST,.value=0,.color={.brt=MAX_SOLID_BRT-6,.r=255,.g=150,.b=10}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,
                                               {.basic={.pattern=LED_SOLID_ST,.value=0,.color={.brt=MAX_SOLID_BRT-6,.r=255,.g=150,.b=10}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_FADE,.val1=600,.val2=NC_MAX_PWM,.t1=10,.t2=8}
                                          },
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,
                                               {.cb={.callback=fp_heightParticle,.data=&(LED_color){.brt=31,.r=0xFF,.g=150,.b=10}}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM}
                                          },
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_PARTICLE,.value=9,.color={.brt=20,.r=255,.g=150,.b=10}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                         }
                                 },
                                 {"shSpark",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_PARTICLE,.value=4,{.brt=5,.r=255,.g=150,.b=10}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,
                                               {.basic={.pattern=LED_PAT_BOOST,.value=0,.color={.brt=7,.r=255,.g=150,.b=10}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedHue,.data=&(LED_color){.brt=MAX_SOLID_BRT-7,.h=30,.s=245,.v=200}}},
                                               {.mode=NC_MODE_INVALID},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,{.cb={.callback=fp_speedHue,.data=&(LED_color){.brt=MAX_SOLID_BRT-7,.h=30,.s=245,.v=200}}},
                                               {.mode=NC_MODE_INVALID},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,
                                               {.cb={.callback=fp_heightParticle,.data=&(LED_color){.brt=31,.r=0xFF,.g=150,.b=10}}},
                                               {.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM},{.mode=NC_MODE_FADE,.val1=600,.val2=NC_MAX_PWM,.t1=10,.t2=8}
                                          },
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_COMPLEX,
                                               {.cb={.callback=fp_heightParticle,.data=&(LED_color){.brt=31,.r=0xFF,.g=150,.b=10}}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=NC_MAX_PWM}
                                          },
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST,
                                               {.basic={.pattern=LED_PAT_PARTICLE,.value=7,.color={.brt=6,.r=255,.g=150,.b=10}}},
                                               {.mode=NC_MODE_PATTERN},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                         }
                                 },
                                 {"day",{
                                          //Startup pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_DAY,.value=14,.color={.brt=31,.r=120,.g=120,.b=120}}},
                                               {.mode=NC_MODE_ONE_SHOT,.val1=NC_MAX_PWM,.val2=0,.t1=8},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Idle pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Pad pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_OFF}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Boost pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_DAY,.value=14,.color={.brt=31,.r=120,.g=120,.b=120}}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Fast pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_DAY,.value=0,.color={.brt=31,.r=0xFF,.g=0,.b=0}}},
                                              {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Coast pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_DAY,.value=0,.color={.brt=31,.r=0xFF,.g=0,.b=0}}},
                                              {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Drogue pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_DAY,.value=0,.color={.brt=31,.r=0,.g=0xFF,.b=0}}},
                                              {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Main pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_DAY,.value=0,.color={.brt=31,.r=0,.g=0,.b=0xFF}}},
                                              {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_FLASH,.val1=NC_MAX_PWM,.val2=0,.t1=100,.t2=500}
                                          },
                                          //Landed pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_DAY,.value=0,.color={.brt=5,.r=0x80,.g=0,.b=0x80}}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Invalid pattern
                                          {FLIGHT_TYPE_PATTERN,{.basic={.pattern=LED_PAT_PATTERN_PANIC}},
                                               {.mode=NC_MODE_STATIC,.val1=0},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                          //Test pattern
                                          {FLIGHT_TYPE_PATTERN_VAL_COLOR,{.basic={.pattern=LED_PAT_COLORTRAIN,.value=0,.color={.brt=8,.r=0xFF,.g=0xFF,.b=0xFF}}},
                                               {.mode=NC_MODE_STATIC,.val1=100},{.mode=NC_MODE_STATIC,.val1=0}
                                          },
                                         }
                                 },
                                 {"",{}}
};

int find_flightP(const char *name)
{
    int i;
    for(i=0;flight_patterns[i].name[0]!='\0';i++)
    {
        if(!strncmp(flight_patterns[i].name,name,FP_NAME_LEN))
        {
            return i;
        }
    }
    //matching pattern not found
    return -1;
}

struct {
            int flying;
        } flightP_dat={0};

int proc_flightP(const struct ao_companion_command *new_dat,const FLIGHT_PATTERN *pattern,int last)
{
    int fm_new=last!=new_dat->flight_state;
    const FLIGHT_MODE_HANDLER *handler;

    if(new_dat->flight_state>=AO_FLIGHT_NUM)
    {
        //set panic mode
        panic(LED_PAT_MODE_PANIC);
        //invalid mode, nothing left to do
        return new_dat->flight_state;
    }

    if(!fm_new && reg_flags&REG_FLAGS_ERROR)
    {
        //if regulator is having problems don't set pattern till flight mode changes
        //this will let the power panic pattern flash and wait for a different flash pattern

        //return current mode
        return new_dat->flight_state;
    }
    //check if we went from flying to not flying
    if(flightP_dat.flying && (new_dat->flight_state<ao_flight_boost || new_dat->flight_state>ao_flight_landed))
    {
        //set panic mode
        panic(LED_PAT_MODE_PANIC);
        //invalid mode, nothing left to do
        return new_dat->flight_state;
    }
    //check if we are flying
    if(new_dat->flight_state>ao_flight_pad && new_dat->flight_state<ao_flight_landed)
    {
        flightP_dat.flying=FP_FLYING;
    }
    //check if we have landed
    if(new_dat->flight_state==ao_flight_landed)
    {
        flightP_dat.flying=FP_NOT_FLYING;
    }

    //get the handler for this mode
    handler=&pattern->handlers[new_dat->flight_state];
    //check if this is a new flight mode
    if(fm_new)
    {
        //set chute and NC
        nosecone_mode(handler->NC_state.mode,handler->NC_state.val1,handler->NC_state.val2,handler->NC_state.t1,handler->NC_state.t2);
        chute_mode(handler->Chute_state.mode,handler->Chute_state.val1,handler->Chute_state.val2,handler->Chute_state.t1,handler->Chute_state.t2);

    }
    //check what type of command we are processing
    switch(handler->type)
    {
        case FLIGHT_TYPE_NO_CHANGE:
        break;
        case FLIGHT_TYPE_PATTERN:
            if(fm_new)
            {
                flashPatternChange(handler->basic.pattern);
            }
        break;
        case FLIGHT_TYPE_PATTERN_VAL_COLOR:
            if(fm_new)
            {
                flashPatternVC(handler->basic.pattern,handler->basic.value,handler->basic.color);
            }
        break;
        case FLIGHT_TYPE_COLOR:
            if(fm_new)
            {
                flashPattern_setColor(handler->basic.color);
            }
        case FLIGHT_TYPE_PATTERN_VAL_COLOR_LIST:
            if(fm_new)
            {
                flashPatternVCL(handler->basic.pattern,handler->basic.value,handler->basic.color,handler->basic.list);
            }
        break;
        case FLIGHT_TYPE_PATTERN_COMPLEX:
            handler->cb.callback(new_dat,fm_new,handler->cb.data);
        break;
        default:
            //not a valid pattern, Panic!!
            panic(LED_PAT_PATTERN_PANIC);
        break;
    }
    //return current mode
    return new_dat->flight_state;
}

void fp_speedGraph(const struct ao_companion_command *cmd_dat,int fm_new, void *color)
{
    static int maxSpeed=0;
    if(fm_new)
    {
        if(cmd_dat->speed>maxSpeed)
        {
            maxSpeed=cmd_dat->speed;
        }
        flashPatternVC(LED_PAT_GRAPH,0,*((LED_color*)color));
    }
    else
    {
        flashPattern_setValue(LED_LEN-(cmd_dat->speed*LED_LEN)/maxSpeed);
    }
    //set nosecone based on speed16-(cmd_dat->height*12)/maxHeight
    nosecone_mode(NC_MODE_STATIC,NC_MAX_PWM*(cmd_dat->speed/(float)maxSpeed),NC_NA,NC_NA,NC_NA);
}

void fp_heightGraph(const struct ao_companion_command *cmd_dat,int fm_new, void *color)
{
    static int maxHeight=0;
    if(fm_new)
    {
        if(cmd_dat->height>maxHeight)
        {
            maxHeight=cmd_dat->height;
        }
        flashPatternVC(LED_PAT_GRAPH,0,*((LED_color*)color));
    }
    else
    {
        flashPattern_setValue((cmd_dat->height*LED_LEN)/maxHeight);
    }
    //set nosecone based on height
    nosecone_mode(NC_MODE_STATIC,NC_MAX_PWM*(cmd_dat->height/(float)maxHeight),NC_NA,NC_NA,NC_NA);
}

void fp_heightParticle(const struct ao_companion_command *cmd_dat,int fm_new, void *color)
{
    static int maxHeight=0;
    if(fm_new)
    {
        if(cmd_dat->height>maxHeight)
        {
            maxHeight=cmd_dat->height;
        }
        flashPatternVC(LED_PAT_UNIFORM_PARTICLE,4,*((LED_color*)color));
    }
    else
    {
        flashPattern_setValue(16-(cmd_dat->height*12)/maxHeight);
    }
}
void fp_heightListParticle(const struct ao_companion_command *cmd_dat,int fm_new, void *list)
{
    static int maxHeight=0;
    if(fm_new)
    {
        if(cmd_dat->height>maxHeight)
        {
            maxHeight=cmd_dat->height;
        }
        flashPatternVCL(LED_PAT_LIST_UNIFORM_PARTICLE,4,(LED_color){.brt=31,.r=0xFF,.g=0xFF,.b=0xFF},(COLOR_LIST*)list);
    }
    else
    {
        flashPattern_setValue(16-(cmd_dat->height*12)/maxHeight);
    }
}

void fp_colorHeight(const struct ao_companion_command *cmd_dat,int fm_new, void *_list)
{
    COLOR_LIST *list=(COLOR_LIST*)_list;
    LED_color color=color=list->alt_color[0].color;
    int i;
    for(i=0;i<list->num_colors;i++)
    {
        if(cmd_dat->height > ((long)list->alt_color[i].alt_frac*(long)settings.alt)>>8)
        {
            color=list->alt_color[i].color;
        }
        if(cmd_dat->height < ((long)list->alt_color[i].alt_frac*(long)settings.alt)>>8)
        {
            break;
        }
    }
    //set brightness
    color.brt=MAX_SINGLE_BRT;

    if(fm_new)
    {
        flashPatternVC(LED_SOLID_ST,0,color);
    }
    else
    {
        flashPattern_setColor(color);
    }
}


void fp_speedHue(const struct ao_companion_command *cmd_dat,int fm_new, void *_hsv)
{
    LED_color color,*hsv=(LED_color*)_hsv;
    static int maxSpeed=0;
    unsigned char hue_shift;
    if(fm_new)
    {
        if(cmd_dat->speed>maxSpeed)
        {
            maxSpeed=cmd_dat->speed;
        }
        //set initial color
        HsvToLED(&color,hsv->brt,hsv->h,hsv->s,hsv->v);
        flashPatternVC(LED_SOLID_ST,0,color);
    }
    else
    {
        hue_shift=0xFF-(unsigned char)((((float)cmd_dat->speed)*0xFF)/((float)maxSpeed));
        HsvToLED(&color,hsv->brt,hsv->h+hue_shift,hsv->s,hsv->v);
        //set new color
        flashPattern_setColor(color);
    }
}
