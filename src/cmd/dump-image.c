// Copyright 2015 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice (including the next
// paragraph) shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util/cru_format.h"
#include "util/cru_image.h"
#include "util/string.h"

#include "cmd.h"

static string_t arg_filename = STRING_INIT;

// From man:getopt(3) :
//
//    By default, getopt() permutes the contents of argv as it scans, so that
//    eventually all the nonoptions are at the end. [...] If the first
//    character of optstring is '+' or the environ‐ ment variable
//    POSIXLY_CORRECT is set, then option processing stops as soon as
//    a nonoption argument is encountered.
//
//    If the first character (following any optional '+' or '-' described
//    above) of optstring is a colon (':'),  then getopt() returns ':' instead
//    of '?' to indicate a missing option argument.
//
static const char *shortopts = "+:h";

static const struct option longopts[] = {
    {"help",          no_argument,       NULL,           'h'},
    {0},
};

static void
parse_args(const cru_command_t *cmd, int argc, char **argv)
{
    // Suppress getopt from printing error messages.
    opterr = 0;

    // Reset getopt.
    optind = 1;

    while (true) {
        int optchar;

        optchar = getopt_long(argc, argv, shortopts, longopts, NULL);

        switch (optchar) {
        case -1:
            goto done_getopt;
	case 0:
	    break;
        case 'h':
            cru_command_page_help(cmd);
            exit(0);
            break;
        case ':':
            cru_usage_error(cmd, "%s requires an argument", argv[optind-1]);
            break;
        case '?':
        default:
            cru_usage_error(cmd, "unknown option: %s", argv[optind-1]);
            break;
        }
    }

done_getopt:
    if (optind == argc)
        cru_usage_error(cmd, "missing <filename>");

    string_copy_cstr(&arg_filename, argv[optind]);
    ++optind;

    if (optind < argc)
        cru_usage_error(cmd, "trailing arguments after <filename>");
}

static noreturn void
die(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    loge_v(format, va);
    va_end(va);

    exit(EXIT_FAILURE);
}

static int
cmd_start(const cru_command_t *cmd, int argc, char **argv)
{
    parse_args(cmd, argc, argv);

    // cru_image_from_filename() interprets relative filenames as relative to
    // Crucible's data directory. That is useful for tests, but not what anyone
    // expects from a cmdline tool.  So convert the filename given on the
    // cmdline into an absolute path.
    string_t abs_filename = STRING_INIT;
    path_to_abs(&abs_filename, &arg_filename);

    cru_image_t *img = cru_image_from_filename(string_data(&abs_filename));
    if (!img)
        exit(EXIT_FAILURE);

    VkFormat format = cru_image_get_format(img);

    const cru_format_info_t *finfo = cru_format_get_info(format);
    if (!finfo)
        die("file has unknown VkFormat %d", format);

    const uint8_t *map = cru_image_map(img, CRU_IMAGE_MAP_ACCESS_READ);
    if (!map)
        die("failed to read file");

    uint32_t width = cru_image_get_width(img);
    uint32_t height = cru_image_get_height(img);
    uint32_t cpp = finfo->cpp;
    uint32_t stride = width * cpp;

    if (width == 0 || height == 0)
        die("file has invalid (width, height) = (%u, %u)", width, height);

    uint32_t y_tick_len = 2 + 2 * (int) log2(height - 1) / 16;

    for (uint32_t y = 0; y < height; ++y) {
        const uint8_t *row = map + y * stride;

        printf("0x%0*u| ", y_tick_len, y);

        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t *pixel = row + x * cpp;

            for (uint32_t c = 0; c < cpp; ++c) {
                printf("%02x", pixel[c]);
            }

            printf(" ");
        }

        printf("\n");
    }

    fflush(stdout);

    return 0;
}

cru_define_command {
    .name = "dump-image",
    .start = cmd_start,
};
