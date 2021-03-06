# -*- coding: utf-8 -*-
##############################################
# The MIT License (MIT)
# Copyright (c) 2014 Kevin Walchko
# see LICENSE for full details
##############################################
import socket


# class GetIP(object):
#     ip = None
#
#     def get(self):
#         s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#         try:
#             # doesn't even have to be reachable
#             s.connect(('10.255.255.255', 1))
#             IP = s.getsockname()[0]
#         except Exception:
#             try:
#                 n = socket.gethostname()
#                 # make sure it has a zeroconfig .local or you end up
#                 # with 127.0.0.1 as your address
#                 if n.find('.local') < 0:
#                     n += '.local'
#                 IP = socket.gethostbyname(n)
#             except Exception:
#                 IP = '127.0.0.1'
#         finally:
#             s.close()
#
#         self.ip = IP
#         return IP


def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    IP = None
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
        try:
            n = socket.gethostname()
            # make sure it has a zeroconfig .local or you end up
            # with 127.0.0.1 as your address
            if n.find('.local') < 0:
                n += '.local'
            IP = socket.gethostbyname(n)
        except Exception:
            IP = '127.0.0.1'
    finally:
        s.close()

    return IP
