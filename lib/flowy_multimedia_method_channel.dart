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
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
