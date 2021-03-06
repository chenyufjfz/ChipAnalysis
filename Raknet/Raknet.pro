#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T15:24:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = Raknet
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += Include

unix {
    target.path = /usr/lib
    INSTALLS += target
}


CONFIG(debug, debug|release) {
DESTDIR = $$_PRO_FILE_PWD_/../lib/debug
} else {
DESTDIR = $$_PRO_FILE_PWD_/../lib/release
}


HEADERS += \
    Include/_FindFirst.h \
    Include/AutopatcherPatchContext.h \
    Include/AutopatcherRepositoryInterface.h \
    Include/Base64Encoder.h \
    Include/BitStream.h \
    Include/CCRakNetSlidingWindow.h \
    Include/CCRakNetUDT.h \
    Include/CheckSum.h \
    Include/CloudClient.h \
    Include/CloudCommon.h \
    Include/CloudServer.h \
    Include/CommandParserInterface.h \
    Include/ConnectionGraph2.h \
    Include/ConsoleServer.h \
    Include/DataCompressor.h \
    Include/DirectoryDeltaTransfer.h \
    Include/DR_SHA1.h \
    Include/DS_BinarySearchTree.h \
    Include/DS_BPlusTree.h \
    Include/DS_BytePool.h \
    Include/DS_ByteQueue.h \
    Include/DS_Hash.h \
    Include/DS_Heap.h \
    Include/DS_HuffmanEncodingTree.h \
    Include/DS_HuffmanEncodingTreeFactory.h \
    Include/DS_HuffmanEncodingTreeNode.h \
    Include/DS_LinkedList.h \
    Include/DS_List.h \
    Include/DS_Map.h \
    Include/DS_MemoryPool.h \
    Include/DS_Multilist.h \
    Include/DS_OrderedChannelHeap.h \
    Include/DS_OrderedList.h \
    Include/DS_Queue.h \
    Include/DS_QueueLinkedList.h \
    Include/DS_RangeList.h \
    Include/DS_Table.h \
    Include/DS_ThreadsafeAllocatingQueue.h \
    Include/DS_Tree.h \
    Include/DS_WeightedGraph.h \
    Include/DynDNS.h \
    Include/EmailSender.h \
    Include/EmptyHeader.h \
    Include/EpochTimeToString.h \
    Include/Export.h \
    Include/FileList.h \
    Include/FileListNodeContext.h \
    Include/FileListTransfer.h \
    Include/FileListTransferCBInterface.h \
    Include/FileOperations.h \
    Include/FormatString.h \
    Include/FullyConnectedMesh2.h \
    Include/Getche.h \
    Include/Gets.h \
    Include/GetTime.h \
    Include/gettimeofday.h \
    Include/GridSectorizer.h \
    Include/HTTPConnection.h \
    Include/HTTPConnection2.h \
    Include/IncrementalReadInterface.h \
    Include/InternalPacket.h \
    Include/Itoa.h \
    Include/Kbhit.h \
    Include/LinuxStrings.h \
    Include/LocklessTypes.h \
    Include/LogCommandParser.h \
    Include/MessageFilter.h \
    Include/MessageIdentifiers.h \
    Include/MTUSize.h \
    Include/NativeFeatureIncludes.h \
    Include/NativeFeatureIncludesOverrides.h \
    Include/NativeTypes.h \
    Include/NatPunchthroughClient.h \
    Include/NatPunchthroughServer.h \
    Include/NatTypeDetectionClient.h \
    Include/NatTypeDetectionCommon.h \
    Include/NatTypeDetectionServer.h \
    Include/NetworkIDManager.h \
    Include/NetworkIDObject.h \
    Include/PacketConsoleLogger.h \
    Include/PacketFileLogger.h \
    Include/PacketizedTCP.h \
    Include/PacketLogger.h \
    Include/PacketOutputWindowLogger.h \
    Include/PacketPool.h \
    Include/PacketPriority.h \
    Include/PluginInterface2.h \
    Include/PS3Includes.h \
    Include/PS4Includes.h \
    Include/Rackspace.h \
    Include/RakAlloca.h \
    Include/RakAssert.h \
    Include/RakMemoryOverride.h \
    Include/RakNetCommandParser.h \
    Include/RakNetDefines.h \
    Include/RakNetDefinesOverrides.h \
    Include/RakNetSmartPtr.h \
    Include/RakNetSocket.h \
    Include/RakNetSocket2.h \
    Include/RakNetStatistics.h \
    Include/RakNetTime.h \
    Include/RakNetTransport2.h \
    Include/RakNetTypes.h \
    Include/RakNetVersion.h \
    Include/RakPeer.h \
    Include/RakPeerInterface.h \
    Include/RakSleep.h \
    Include/RakString.h \
    Include/RakThread.h \
    Include/RakWString.h \
    Include/Rand.h \
    Include/RandSync.h \
    Include/ReadyEvent.h \
    Include/RefCountedObj.h \
    Include/RelayPlugin.h \
    Include/ReliabilityLayer.h \
    Include/ReplicaEnums.h \
    Include/ReplicaManager3.h \
    Include/Router2.h \
    Include/RPC4Plugin.h \
    Include/SecureHandshake.h \
    Include/SendToThread.h \
    Include/SignaledEvent.h \
    Include/SimpleMutex.h \
    Include/SimpleTCPServer.h \
    Include/SingleProducerConsumer.h \
    Include/SocketDefines.h \
    Include/SocketIncludes.h \
    Include/SocketLayer.h \
    Include/StatisticsHistory.h \
    Include/StringCompressor.h \
    Include/StringTable.h \
    Include/SuperFastHash.h \
    Include/TableSerializer.h \
    Include/TCPInterface.h \
    Include/TeamBalancer.h \
    Include/TeamManager.h \
    Include/TelnetTransport.h \
    Include/ThreadPool.h \
    Include/ThreadsafePacketLogger.h \
    Include/TransportInterface.h \
    Include/TwoWayAuthentication.h \
    Include/UDPForwarder.h \
    Include/UDPProxyClient.h \
    Include/UDPProxyCommon.h \
    Include/UDPProxyCoordinator.h \
    Include/UDPProxyServer.h \
    Include/VariableDeltaSerializer.h \
    Include/VariableListDeltaTracker.h \
    Include/VariadicSQLParser.h \
    Include/VitaIncludes.h \
    Include/WindowsIncludes.h \
    Include/WSAStartupSingleton.h \
    Include/XBox360Includes.h

SOURCES += \
    Source/_FindFirst.cpp \
    Source/Base64Encoder.cpp \
    Source/BitStream.cpp \
    Source/CCRakNetSlidingWindow.cpp \
    Source/CCRakNetUDT.cpp \
    Source/CheckSum.cpp \
    Source/CloudClient.cpp \
    Source/CloudCommon.cpp \
    Source/CloudServer.cpp \
    Source/CommandParserInterface.cpp \
    Source/ConnectionGraph2.cpp \
    Source/ConsoleServer.cpp \
    Source/DataCompressor.cpp \
    Source/DirectoryDeltaTransfer.cpp \
    Source/DR_SHA1.cpp \
    Source/DS_BytePool.cpp \
    Source/DS_ByteQueue.cpp \
    Source/DS_HuffmanEncodingTree.cpp \
    Source/DS_Table.cpp \
    Source/DynDNS.cpp \
    Source/EmailSender.cpp \
    Source/EpochTimeToString.cpp \
    Source/FileList.cpp \
    Source/FileListTransfer.cpp \
    Source/FileOperations.cpp \
    Source/FormatString.cpp \
    Source/FullyConnectedMesh2.cpp \
    Source/Getche.cpp \
    Source/Gets.cpp \
    Source/GetTime.cpp \
    Source/gettimeofday.cpp \
    Source/GridSectorizer.cpp \
    Source/HTTPConnection.cpp \
    Source/HTTPConnection2.cpp \
    Source/IncrementalReadInterface.cpp \
    Source/Itoa.cpp \
    Source/LinuxStrings.cpp \
    Source/LocklessTypes.cpp \
    Source/LogCommandParser.cpp \
    Source/MessageFilter.cpp \
    Source/NatPunchthroughClient.cpp \
    Source/NatPunchthroughServer.cpp \
    Source/NatTypeDetectionClient.cpp \
    Source/NatTypeDetectionCommon.cpp \
    Source/NatTypeDetectionServer.cpp \
    Source/NetworkIDManager.cpp \
    Source/NetworkIDObject.cpp \
    Source/PacketConsoleLogger.cpp \
    Source/PacketFileLogger.cpp \
    Source/PacketizedTCP.cpp \
    Source/PacketLogger.cpp \
    Source/PacketOutputWindowLogger.cpp \
    Source/PluginInterface2.cpp \
    Source/PS4Includes.cpp \
    Source/Rackspace.cpp \
    Source/RakMemoryOverride.cpp \
    Source/RakNetCommandParser.cpp \
    Source/RakNetSocket.cpp \
    Source/RakNetSocket2.cpp \
    Source/RakNetSocket2_360_720.cpp \
    Source/RakNetSocket2_Berkley.cpp \
    Source/RakNetSocket2_Berkley_NativeClient.cpp \
    Source/RakNetSocket2_NativeClient.cpp \
    Source/RakNetSocket2_PS3_PS4.cpp \
    Source/RakNetSocket2_PS4.cpp \
    Source/RakNetSocket2_Vita.cpp \
    Source/RakNetSocket2_Windows_Linux.cpp \
    Source/RakNetSocket2_Windows_Linux_360.cpp \
    Source/RakNetSocket2_WindowsStore8.cpp \
    Source/RakNetStatistics.cpp \
    Source/RakNetTransport2.cpp \
    Source/RakNetTypes.cpp \
    Source/RakPeer.cpp \
    Source/RakSleep.cpp \
    Source/RakString.cpp \
    Source/RakThread.cpp \
    Source/RakWString.cpp \
    Source/Rand.cpp \
    Source/RandSync.cpp \
    Source/ReadyEvent.cpp \
    Source/RelayPlugin.cpp \
    Source/ReliabilityLayer.cpp \
    Source/ReplicaManager3.cpp \
    Source/Router2.cpp \
    Source/RPC4Plugin.cpp \
    Source/SecureHandshake.cpp \
    Source/SendToThread.cpp \
    Source/SignaledEvent.cpp \
    Source/SimpleMutex.cpp \
    Source/SocketLayer.cpp \
    Source/StatisticsHistory.cpp \
    Source/StringCompressor.cpp \
    Source/StringTable.cpp \
    Source/SuperFastHash.cpp \
    Source/TableSerializer.cpp \
    Source/TCPInterface.cpp \
    Source/TeamBalancer.cpp \
    Source/TeamManager.cpp \
    Source/TelnetTransport.cpp \
    Source/ThreadsafePacketLogger.cpp \
    Source/TwoWayAuthentication.cpp \
    Source/UDPForwarder.cpp \
    Source/UDPProxyClient.cpp \
    Source/UDPProxyCoordinator.cpp \
    Source/UDPProxyServer.cpp \
    Source/VariableDeltaSerializer.cpp \
    Source/VariableListDeltaTracker.cpp \
    Source/VariadicSQLParser.cpp \
    Source/VitaIncludes.cpp \
    Source/WSAStartupSingleton.cpp
