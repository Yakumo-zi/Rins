#include "rins_api.h"
#include "rins.pb.h"
#include <bits/stdint-uintn.h>

rins_client::rins_client() : _seqid(0) {
    printf("rins_client()\n");
    // 1 初始化服务器地址
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    //默认的ip地址是本地，因为是API和agent应该部署于同一台主机上
    inet_aton("127.0.0.1", &servaddr.sin_addr);
    // 2. 创建3个UDPsocket
    for (int i = 0; i < 3; i++) {
        _sockfd[i] = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
        if (_sockfd[i] == -1) {
            perror("socket()");
            exit(1);
        }
        servaddr.sin_port = htons(8888 + i);
        int ret = connect(_sockfd[i], (const struct sockaddr *)&servaddr,
                          sizeof(servaddr));
        if (ret == -1) {
            perror("connect()");
            exit(1);
        }
        printf("connection agent udp server succ!\n");
    }
}

rins_client::~rins_client() {
    printf("~rins_client()\n");

    for (int i = 0; i < 3; ++i) {
        close(_sockfd[i]);
    }
}

int rins_client::get_host(int modid, int cmdid, std::string &ip, int port) {
    uint32_t seq = _seqid++;
    rins::GetHostRequest req;
    req.set_seq(seq);
    req.set_modid(modid);
    req.set_cmdid(cmdid);

    char write_buf[4096], read_buf[80 * 1024];
    msg_head head;
    head.length = req.ByteSizeLong();
    head.id = rins::ID_GetHostRequest;
    memcpy(write_buf, &head, MESSAGE_HEAD_LEN);
    req.SerializeToArray(write_buf + MESSAGE_HEAD_LEN, head.length);

    int index = (modid + cmdid) % 3;
    int ret = sendto(_sockfd[index], write_buf, head.length + MESSAGE_HEAD_LEN,
                     0, NULL, 0);
    if (ret == -1) {
        perror("sendto");
        return rins::RET_SYSTEM_ERROR;
    }
    int message_len;
    rins::GetHostResponse resp;

    do {
        message_len =
            recvfrom(_sockfd[index], read_buf, sizeof(read_buf), 0, NULL, NULL);
        if (message_len == -1) {
            perror("recvfrom");
            return rins::RET_SYSTEM_ERROR;
        }

        //消息头
        memcpy(&head, read_buf, MESSAGE_HEAD_LEN);
        if (head.id != rins::ID_GetHostResponse) {
            fprintf(stderr, "message ID error!\n");
            return rins::RET_SYSTEM_ERROR;
        }

        //消息体
        ret = resp.ParseFromArray(read_buf + MESSAGE_HEAD_LEN,
                                  message_len - MESSAGE_HEAD_LEN);
        if (!ret) {
            fprintf(stderr,
                    "message format error: head.msglen = %d, message_len = %d, "
                    "message_len - MESSAGE_HEAD_LEN = %d, head msgid = %d, "
                    "ID_GetHostResponse = %d\n",
                    head.length, message_len, message_len - MESSAGE_HEAD_LEN,
                    head.length, rins::ID_GetHostResponse);
            return rins::RET_SYSTEM_ERROR;
        }
    } while (resp.seq() < seq);

    if (resp.seq() != seq || resp.modid() != modid || resp.cmdid() != cmdid) {
        fprintf(stderr, "message format error\n");
        return rins::RET_SYSTEM_ERROR;
    }

    // 4 处理消息
    if (resp.retcode() == 0) {
        rins::HostInfo host = resp.host();

        in_addr inaddr;
        inaddr.s_addr = host.ip();
        ip = inet_ntoa(inaddr);
        port = host.port();
    }

    return resp.retcode(); // rins::RET_SUCC
}