import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'flowy_multimedia_platform_interface.dart';

/// An implementation of [FlowyMultimediaPlatform] that uses method channels.
class MethodChannelFlowyMultimedia extends FlowyMultimediaPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('flowy_multimedia');

  @override
  Future<String?> getPlatformVersion() async {
    final version =
        await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }

  @override
  Future<int> subscribeToRoom(String roomId) async {
    int? textureId = await methodChannel.invokeMethod<int>('subscribeToRoom');
    if (textureId == null) {
      throw Exception('textureId is null');
    }

    return textureId;
  }

  @override
  Future<int> startPublish() async {
    int? textureId = await methodChannel.invokeMethod<int>('startPublish');

    if (textureId == null) {
      throw Exception('textureId is null');
    }

    return textureId;
  }

  @override
  Future<void> stopPublish() async {
    await methodChannel.invokeMethod<void>('stopPublish');
  }

  @override
  Future<void> startRecord() async {
    await methodChannel.invokeMethod<void>('startRecord');
  }

  @override
  Future<String> stopRecord() async {
    await methodChannel.invokeMethod<void>('stopRecord');
    return '';
  }
}
