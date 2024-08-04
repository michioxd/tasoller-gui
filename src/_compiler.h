#pragma once

#ifndef __packed
#if defined(__CC_ARM)
#elif defined(__GNUC__)
#define __packed __attribute__((packed))
#else
#error Unknown compiler
#endif
#endif
