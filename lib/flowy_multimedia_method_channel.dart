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
  Future<String?> init() async {
    final result = await methodChannel.invokeMethod<String>('init');
    return result;
  }

  @override
  Future<String?> startSendLiveAudio() async {
    final result =
        await methodChannel.invokeMethod<String>('startSendLiveAudio');
    return result;
  }

  @override
  Future<String?> stopSendLiveAudio() async {
    final result =
        await methodChannel.invokeMethod<String>('stopSendLiveAudio');
    return result;
  }

  @override
  Future<String?> startRecord() async {
    final result = await methodChannel.invokeMethod<String>('startRecord');
    return result;
  }

  @override
  Future<String?> stopRecord() async {
    final result = await methodChannel.invokeMethod<String>('stopRecord');
    return result;
  }
}
