/* additional client events */

#define E_CONNECT_NEW 0x0201
#define E_CONNECT_FAVORITE 0x0202
#define E_DISCONNECT 0x0203
#define E_SERVER 0x0204
#define E_SERVER_LIST 0x0205
#define E_SERVER_ADD_FAVORITE 0x0206
#define E_SERVER_EDIT_FAVORITE 0x0207

#define E_IDENTITY 0x0301
#define E_OPTIONS 0x0302
#define E_DCC_OPTIONS 0x0303
#define E_DCC_SEND_OPTIONS 0x0304
#define E_SCRIPT_OPTIONS 0x0305
#define E_CTCP_OPTIONS 0x0306
#define E_COLOR_OPTIONS 0x0307
#define E_TERM_INFO 0x0308
#define E_MISC_OPTIONS 0x0309
#define E_NETWORK_OPTIONS 0x310
#define E_SAVE_OPTIONS 0x0320

#define E_CHANNEL_JOIN 0x0501
#define E_JOIN_NEW 0x0502
#define E_JOIN_FAVORITE 0x0503

#define E_CHANNEL_PART 0x0504
#define E_CHANNEL_ADD_FAVORITE 0x0505
#define E_CHANNEL_EDIT_FAVORITE 0x0506
#define E_CHANNEL_LIST 0x0507

#define E_CHAT_NEW 0x0601
#define E_CHAT_FAVORITE 0x0602

#define E_DCC_CHAT 0x0603
#define E_DCC_SEND 0x0604
#define E_DCC_CHAT_NEW 0x0605
#define E_DCC_CHAT_FAVORITE 0x0606
#define E_DCC_SEND_NEW 0x0607
#define E_DCC_SEND_FAVORITE 0x0608

#define E_USER_SELECT 0x15
#define E_USER_ADD_FAVORITE 0x0618
#define E_USER_ADD_IGNORE 0x0619
#define E_USER_EDIT_FAVORITE 0x061a
#define E_USER_EDIT_IGNORED 0x061b
#define E_USER_REMOVE_FAVORITE 0x061c
#define E_USER_REMOVE_IGNORED 0x061d

#define E_HELP_IRC_COMMANDS 0x0401
#define E_HELP_KEYS 0x0402
#define E_HELP_COMMANDS 0x0403
#define E_HELP_ABOUT 0x0404

#define E_USER_PASTE 0x8000

#define E_CTCP_PING 0x9000
#define E_CTCP_CLIENTINFO 0x9001
#define E_CTCP_USERINFO 0x9002
#define E_CTCP_VERSION 0x9003
#define E_CTCP_FINGER 0x9004
#define E_CTCP_SOURCE 0x9005
#define E_CTCP_TIME 0x9006

#define E_CONTROL_OP 0x9100
#define E_CONTROL_DEOP 0x9101
#define E_CONTROL_VOICE 0x9102
#define E_CONTROL_DEVOICE 0x9103
#define E_CONTROL_KICK 0x9104
#define E_CONTROL_BAN 0x9105
#define E_CONTROL_KICKBAN 0x9106

#define E_WHOIS 0xa000
#define E_QUERY 0xa001

#define E_TRANSFER_INFO 0xb000
#define E_TRANSFER_STOP 0xb001

#define E_EXIT_LASTSERVER 0xc000
#define E_CLOSE_SERVER 0xc001

#define E_LIST_VIEW 0xb000

