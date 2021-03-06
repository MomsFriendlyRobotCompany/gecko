##############################################
# The MIT License (MIT)
# Copyright (c) 2018 Kevin Walchko
# see LICENSE for full details
##############################################
#
# https://pymotw.com/2/socket/multicast.html
#
import socket
import struct
import threading
import time
# import ipaddress  # kjw
from ip import GetIP
from transport import Ascii, Json, Pickle
import os


class BeaconBase(object):
    """
    https://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
    TTL  Scope
    ----------------------------------------------------------------------
       0 Restricted to the same host. Won't be output by any interface.
       1 Restricted to the same subnet. Won't be forwarded by a router.
     <32 Restricted to the same site, organization or department.
     <64 Restricted to the same region.
    <128 Restricted to the same continent.
    <255 Unrestricted in scope. Global.
    """
    mcast_addr = '224.3.29.110'
    mcast_port = 11311
    timeout = 2
    ttl = 1

    def __init__(self, key, ttl=1):
        self.group = (self.mcast_addr, self.mcast_port)
        self.sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM,socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_TTL, ttl)
        self.sock.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_LOOP, 1)
        self.key = key


class BeaconCoreServer(BeaconBase):
    services = {}  # services
    perf = {}
    exit = False

    def __init__(self, key, handler=Ascii, ttl=1, addr=None):
        BeaconBase.__init__(self, key=key, ttl=ttl)

        if addr is not None:
            if len(addr) == 2:
                self.mcast_addr = addr[0]
                self.mcast_port = addr[1]

        # setup service socket
        # allow multiple connections
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            self.sock.bind(('0.0.0.0', self.mcast_port))
        except OSError as e:
            print("*** {} ***".format(e))
            raise

        mreq = struct.pack("=4sl", socket.inet_aton(self.mcast_addr), socket.INADDR_ANY)
        self.sock.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP, mreq)

        # setup server data
        self.handler = handler()  # serialization method

        # setup thread
        self.listener = threading.Thread(target=self.printLoop)

    def printLoop(self):
        while not self.exit:
            self.print()
            time.sleep(3)

    def start(self):
        """Start the listener thread"""
        self.listener.setDaemon(True)
        self.listener.start()

    def stop(self):
        """Stop the listener thread"""
        self.exit = True

    def print(self):
        print(" ")
        print("="*40)
        print(" Listening on: {}:{}".format(self.mcast_addr, self.mcast_port))
        print('Known Services', '-'*40)
        for k,v in self.services.items():
            print(" * {}: {}".format(k,v))
        print('\nPerformance', '-'*40)
        for k,v in self.perf.items():
            print(" * {}: {}".format(k,v))
        print(" ")

    def handle_sub(self, data):
        ret = None
        topic = data[1]
        pid = data[2]
        if topic in self.services.keys():
            endpt = self.services[topic]
            ret = (self.key, topic, endpt,)
            self.perf[topic] = pid

            print(">> SUB[{}] {}:{}".format(pid, topic, endpt))

        return ret

    def handle_pub(self, data):
        topic = data[1]
        pid = int(data[2])
        endpt = data[3]

        self.services[topic] = endpt
        self.perf[topic] = pid

        print(">> PUB[{}] {}:{}".format(pid,topic,endpt))

        return (self.key,topic,endpt,"ok",)

    def callback(self, data, address):
        # print(">> Address: {}".format(address))
        # print(">> Data: {} size: {}".format(data, len(data)))

        ret = None

        if self.key == data[0]:
            if len(data) == 3: ret = self.handle_sub(data)
            elif len(data) == 4: ret = self.handle_pub(data)
        else:
            printf("** Invalid key:", data)

        return ret

    def run(self):
        # self.sock.setblocking(0)

        ip = GetIP().get()
        print("<<< beacon ip: {} >>>".format(ip))

        while True:
            try:
                data, address = self.sock.recvfrom(1024)
                data = self.handler.loads(data)

                if self.key == data[0]:
                    msg = self.callback(data, address)
                    if msg:
                        msg  = self.handler.dumps(msg)
                        self.sock.sendto(msg, address)
                        # print(">> beacon sent: {}".format(msg))

            except KeyboardInterrupt:
                print("ctrl-z")
                self.exit = True
                return
            except Exception:
                continue



######################################################


# class BeaconFinder(BeaconBase):
#     """
#     Find Services using the magic of multicast
#
#     pid = 123456
#     proc_name = "my-cool-process"
#     key = hostname
#     finder = BeaconFinder(key)
#     msg = finder.search(msg)
#     """
#     def __init__(self, key, ttl=1, handler=Pickle):
#         BeaconBase.__init__(self, key=key, ttl=ttl)
#         self.handler = handler()
#
#     def send(self, msg):
#         """
#         Search for services using multicast sends out a request for services
#         of the specified name and then waits and gathers responses. This sends
#         one mdns ping. As soon as a responce is received, the function returns.
#         """
#         # serviceName = 'GeckoCore'
#         self.sock.settimeout(self.timeout)
#         # msg = self.handler.dumps((self.key, serviceName, str(pid), processname,))
#         # msg['key'] = self.key
#         msg = self.handler.dumps(msg)
#         self.sock.sendto(msg, self.group)
#         servicesFound = None
#         while True:
#             try:
#                 # data = returned message info
#                 # server = ip:port, which is x.x.x.x:9990
#                 data, server = self.sock.recvfrom(1024)
#                 data = self.handler.loads(data)
#                 # print('>> Search:', data, server)
#                 servicesFound = data
#                 break
#                 # if len(data) == 2:
#                 #     servicesFound = (zmqTCP(server[0], data[0]), zmqTCP(server[0], data[1]),)
#                 #     break
#             except socket.timeout:
#                 print("*** timeout ***")
#                 break
#         # print(">> search done")
#         return servicesFound

#
# class BeaconServer(BeaconBase):
#     """A simple multicast listener which responds to
#     requests for services it has
#
#     # message to be transmitted via multicast
#     msg = {'something': 123, 'other': 'abc'}
#
#     # create a server
#     provider = BeaconServer(
#         'hostname',
#         callback_function [optional],  # ??
#         handler              # ??
#     )
#
#     provider.start()
#     try:
#         while True:
#             time.sleep(500)
#     except KeyboardInterrupt:
#         provider.stop()
#
#     """
#     def __init__(self, key, callback=None, handler=Ascii, ttl=1, addr=None):
#         BeaconBase.__init__(self, key=key, ttl=ttl)
#
#         if addr is not None:
#             if len(addr) == 2:
#                 self.mcast_addr = addr[0]
#                 self.mcast_port = addr[1]
#
#         # setup service socket
#         # allow multiple connections
#         self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#         try:
#             self.sock.bind(('0.0.0.0', self.mcast_port))
#             # self.sock.bind((self.mcast_addr, self.mcast_port))
#         except OSError as e:
#             print("*** {} ***".format(e))
#             raise
#
#         mreq = struct.pack("=4sl", socket.inet_aton(self.mcast_addr), socket.INADDR_ANY)
#         self.sock.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP, mreq)
#
#         # setup server data
#         # self.services = {}  # services
#         self.callback = callback
#         self.handler = handler()  # serialization method
#
#         # setup thread
#         # self.exit = False
#         # self.listener = threading.Thread(target=self.listenerThread)
#
#     # def start(self):
#     #     """Start the listener thread"""
#     #     self.listener.setDaemon(True)
#     #     self.listener.start()
#     #
#     # def stop(self):
#     #     """Stop the listener thread"""
#     #     self.exit = True
#     #
#     # def listen(self):
#     #     """TBD"""
#     #     pass
#
#     def run(self):
#         # self.sock.setblocking(0)
#
#         ip = GetIP().get()
#         print("<<< beacon ip: {} >>>".format(ip))
#
#         # while self.exit is False:
#         while True:
#             # print('-'*40)
#             # for k,v in self.services.items():
#             #     print("{}: {}".format(k,v))
#             #
#             # if self.exit is True:
#             #     break
#             # else:
#             # time.sleep(0.2)
#             try:
#                 # time.sleep(0.2)
#                 data, address = self.sock.recvfrom(1024)
#             except KeyboardInterrupt:
#                 print("ctrl-z")
#                 return
#             except Exception:
#                 continue
#
#             data = self.handler.loads(data)
#             # print(">> Address: {}".format(address))
#             # print(">> Data: {}".format(data))
#
#             if self.key == data[0]:
#                 if self.callback:
#                     msg = self.callback(data, address)
#                     # print("MM:",msg)
#                     if msg:
#                         msg  = self.handler.dumps(msg)
#                         self.sock.sendto(msg, address)
#                         # print(">> beacon sent: {}".format(msg))
#                 # else:
#                 #     msg  = self.handler.dumps(('hello',))
#                 #     self.sock.sendto(msg, address)
