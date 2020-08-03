/*
 * Copyright 2013 Artem Harutyunyan, Sergey Shekyan
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "camtool.h"
#include "common.h"

static void
info()
{
    fprintf(stdout, "Try \'confextract -h\' for more information\n");
}

static void
usage()
{
    fprintf(stdout,
            "Extraction/integrity check tool for camera configuration.\n"
            "Usage: confextract [-cx] <input file> -o <output dir>\n"
            "\t-c <file>        perform integrity check of <file>\n"
            "\t-x <file>        extract <file>. Default destination is current working dir\n"
            "\t-o <output file>  output to <output file>\n");
}

int32_t
conf_extract_conf(conf_file *conf, FILE * f)
{
    // header
    fprintf(f, "magic::::%#x\n", conf->header.magic);
    fprintf(f, "checksum::::%#x\n", conf->header.checksum);
    fprintf(f, "reserve::::%#x\n", conf->header.reserve);
    fprintf(f, "camid::::%s\n", conf->header.camid);
    fprintf(f, "system version::::%#x\n", conf->header.sysver);
    fprintf(f, "webui version::::%#x\n", conf->header.webuiver);
    fprintf(f, "alias::::%s\n", conf->header.alias);

    // users
    int32_t i;
    for(i = 0; i < 8; ++i) {
        fprintf(f, "username%d::::%s\n", i + 1,  conf->users[i].username);
        fprintf(f, "password%d::::%s\n", i + 1, conf->users[i].password);
        fprintf(f, "role%d::::%d\n", i + 1, conf->users[i].role);
    }
    // network
    fprintf(f, "ipaddr::::%d\n", conf->network.ipaddr);
    fprintf(f, "mask::::%d\n", conf->network.mask);
    fprintf(f, "gateway::::%d\n", conf->network.gateway);
    fprintf(f, "dns::::%d\n", conf->network.dns);
    fprintf(f, "unknown::::%d\n", conf->network.unknown);
    fprintf(f, "port::::%d\n", conf->network.port);

    // adsl
    fprintf(f, "attr::::%d\n", conf->adsl.attr);
    fprintf(f, "adsl_username::::%s\n", conf->adsl.username);
    fprintf(f, "adsl_password::::%s\n", conf->adsl.password);

    // wifi
    fprintf(f, "wifi_enable::::%d\n", conf->wifi.enable & 1);
    fprintf(f, "wifi_ssid::::%s\n", conf->wifi.ssid);
    fprintf(f, "wifi_wpa_psk::::%s\n", conf->wifi.wpa_psk);
    fprintf(f, "wifi_country::::%d\n", conf->wifi.country & 1);
    // e-mail
    fprintf(f, "email_notify_ip_change::::%d\n", conf->email.mail_inet_ip & 1);
    fprintf(f, "email_sender::::%s\n", conf->email.sender);
    fprintf(f, "email_receiver1::::%s\n", conf->email.receiver1);
    fprintf(f, "email_receiver2::::%s\n", conf->email.receiver2);
    fprintf(f, "email_receiver3::::%s\n", conf->email.receiver3);
    fprintf(f, "email_receiver4::::%s\n", conf->email.receiver4);
    fprintf(f, "email_server::::%s\n", conf->email.mail_server);
    fprintf(f, "email_port::::%hd\n", conf->email.port);
    fprintf(f, "email_username::::%s\n", conf->email.username);
    fprintf(f, "email_password::::%s\n", conf->email.password);

    return 1;
}
int32_t
conf_read_header(FILE * f, const conf_header_offset_t type,
                 conf_file_header * file_header)
{
    int32_t retval = 1;
    fseek(f, conf_header_field[type], SEEK_SET);
    switch (type) {
    case CONF_OFFSET_MAGIC:
        fread(&file_header->magic, 1, 4, f);
        // magic number
        break;
    case CONF_OFFSET_CHECKSUM:
        fread(&file_header->checksum, 1, 4, f);
        // declared checksum
        break;
    case CONF_OFFSET_RESERVE:
        fread(&file_header->reserve, 1, 4, f);
        // don't know what is this constant value
        break;
    case CONF_OFFSET_CAMID:
        fread(&file_header->camid, 1, 13, f);
        // camera ID
        break;
    case CONF_OFFSET_SYS_VER:
        fread(&file_header->sysver, 1, 4, f);
        // system version
        break;
    case CONF_OFFSET_UI_VER:
        fread(&file_header->webuiver, 1, 4, f);
        // WebUI firmware version
        break;
    case CONF_OFFSET_ALIAS:
        fread(&file_header->alias, 1, 21, f);
        break;
    default:
        retval = 0;
        break;
    }
    if (feof(f)) {
        retval = 0;
    } else if (ferror(f)) {
        fprintf(stderr, "Error reading file: %s\n", strerror(errno));
        retval = 0;
    }
    return retval;
}

int32_t
conf_read_users(FILE * f, conf_file * conf)
{
    int32_t retval = 1;
    int32_t i;
    for(i = 0; i < 8; ++i) {
        if(fread(&conf->users[i], 1, sizeof(conf_user), f) != sizeof(conf_user)){
            fprintf(stderr, "error reading users section\n.");
            return 0;
        }
    }
    return retval;
}

int32_t
conf_read_sections(FILE * f, const conf_sections_offset_t type,
                   conf_file * conf)
{
    int32_t retval = 1;
    fseek(f, conf_sections_field[type], SEEK_SET);
    switch (type) {
    case CONF_OFFSET_MAGIC:
        retval = conf_read_users(f, conf);
        // read users section
        break;
    case CONF_OFFSET_NETWORK:
        if(fread(&conf->network, 1, sizeof(struct conf_network_t), f) != sizeof(struct conf_network_t)) {
            fprintf(stderr, "error reading network section\n.");
            return 0;
        }
        break;
    case CONF_OFFSET_ADSL:
        if(fread(&conf->adsl, 1, sizeof(conf_adsl), f) != sizeof(conf_adsl)) {
            fprintf(stderr, "error reading ADSL section\n.");
            return 0;
        }
        break;
    case CONF_OFFSET_WIFI:
        if(fread(&conf->wifi, 1, sizeof(conf_wifi), f) != sizeof(conf_wifi)) {
            fprintf(stderr, "error reading WIFI section\n.");
            return 0;
        }
        break;
    case CONF_OFFSET_EMAIL:
        if(fread(&conf->email, 1, sizeof(conf_email), f) != sizeof(conf_email)) {
            fprintf(stderr, "error reading e-mail section\n.");
            return 0;
        }
        break;
    default:
        retval = 0;
        break;
    }
    return retval;
}
int32_t
conf_validate_file(FILE * f, conf_file * conf)
{
    int32_t retval = 1;
    if (!conf_read_header(f, CONF_OFFSET_MAGIC, &conf->header))
        return 0;
    if (!conf_read_header(f, CONF_OFFSET_CHECKSUM, &conf->header))
        return 0;
    if (!conf_read_header(f, CONF_OFFSET_RESERVE, &conf->header))
        return 0;
    if (!conf_read_header(f, CONF_OFFSET_CAMID, &conf->header))
        return 0;
    if (!conf_read_header(f, CONF_OFFSET_SYS_VER, &conf->header))
        return 0;
    if (!conf_read_header(f, CONF_OFFSET_UI_VER, &conf->header))
        return 0;
    if (!conf_read_header(f, CONF_OFFSET_ALIAS, &conf->header))
        return 0;
    if(!conf_read_sections(f, CONF_OFFSET_USERS, conf))
        return 0;
    if(!conf_read_sections(f, CONF_OFFSET_NETWORK, conf))
        return 0;
    if(!conf_read_sections(f, CONF_OFFSET_ADSL, conf))
        return 0;
    if(!conf_read_sections(f, CONF_OFFSET_WIFI, conf))
        return 0;
    if(!conf_read_sections(f, CONF_OFFSET_EMAIL, conf))
        return 0;

    if(conf->header.magic != CONF_MAGIC) {
        fprintf(stderr,
                "Declared file magic number doesn't match the known number: %#x/%#x\n",
                conf->header.magic, CONF_MAGIC);
        retval = 0;
    }
    int32_t checksum = calc_checksum_file(f, conf_header_field[CONF_OFFSET_CAMID]);
    if(conf->header.checksum != checksum) {
        fprintf(stderr,
                "Declared checksum doesn't match the calculated checksum: %#x/%#x\n",
                conf->header.checksum, checksum);
        retval = 0;
    }
    return retval;
}
int32_t
main(int argc, char **argv)
{

    if (argc < 2) {
        usage();
        return 1;
    }
    char      o;
    int32_t   validate_only = 0;
    int32_t   index;
    char      in_file_name[MAX_FILE_NAME_LEN] = { 0 };
    char      dst_file_name[MAX_FILE_NAME_LEN] = { 0 };
    while ((o = getopt(argc, argv, ":c:x:ho:")) != -1) {
        switch (o) {
        case 'c':
            validate_only = 1;
            strncpy(in_file_name, optarg, MAX_FILE_NAME_LEN);
            break;
        case 'x':
            strncpy(in_file_name, optarg, MAX_FILE_NAME_LEN);
            break;
        case 'h':
            usage();
            return 0;
        case 'o':
            strncpy(dst_file_name, optarg, MAX_FILE_NAME_LEN);
            break;
        case '?':
            fprintf(stderr, "Illegal option -%c\n", optopt);
            info();
            return 1;
        default:
            fprintf(stderr, "Option -%c requires an argument.\n",
                    optopt);
            info();
            return 1;
        }
    }

    for (index = optind; index < argc; index++) {
        fprintf(stderr, "Non-option argument %s\n", argv[index]);
        info();
        return 1;
    }

    if (!strlen(dst_file_name)) {
        strncpy(dst_file_name, DEFAULT_FILE, sizeof(DEFAULT_FILE));
    }
    FILE  *file = fopen(in_file_name, "rb");
    if (!file) {
        fprintf(stderr, "Error opening file %s: %s\n", in_file_name,
                strerror(errno));
        return 1;
    }
    conf_file conf = { 0 };
    if (!conf_validate_file(file, &conf)) {
        return 1;
    }
    if (!validate_only) {
        FILE  *dst_file = fopen(dst_file_name, "wb");
        if (file == NULL) {
            fprintf(stderr, "Unable to write file %s: %s\n",
                    dst_file_name, strerror(errno));
            return 1;
        } else {
            fprintf(stdout, "Created output file %s\n", dst_file_name);
        }
        conf_extract_conf(&conf, dst_file);
        fclose(dst_file);
    }
    fclose(file);
    fprintf(stdout,
            "\nIntegrity check passed:\n"
            "Magic number \t%#x\n"
            "Checksum     \t%#x\n"
            "Camera ID    \t%s\n"
            "Sys version  \t%d.%d.%d.%d\n"
            "WebUI version\t%d.%d.%d.%d\n"
            "Alias        \t%s\n" , conf.header.magic,
            conf.header.checksum,
            conf.header.camid, (conf.header.sysver >> (0 * 8)) & 0xFF,
            (conf.header.sysver >> (1 * 8)) & 0xFF,
            (conf.header.sysver >> (2 * 8)) & 0xFF,
            (conf.header.sysver >> (3 * 8)) & 0xFF,
            (conf.header.webuiver >> (0 * 8)) & 0xFF,
            (conf.header.webuiver >> (1 * 8)) & 0xFF,
            (conf.header.webuiver >> (2 * 8)) & 0xFF,
            (conf.header.webuiver >> (3 * 8)) & 0xFF,
            conf.header.alias);
    return 0;
}

