import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'flowy_multimedia_method_channel.dart';

abstract class FlowyMultimediaPlatform extends PlatformInterface {
  /// Constructs a FlowyMultimediaPlatform.
  FlowyMultimediaPlatform() : super(token: _token);

  static final Object _token = Object();

  static FlowyMultimediaPlatform _instance = MethodChannelFlowyMultimedia();

  /// The default instance of [FlowyMultimediaPlatform] to use.
  ///
  /// Defaults to [MethodChannelFlowyMultimedia].
  static FlowyMultimediaPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [FlowyMultimediaPlatform] when
  /// they register themselves.
  static set instance(FlowyMultimediaPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }

  /// subscribe to media from the room with the given roomId
  /// any publish will be done to this room as well
  /// @returns textureId
  Future<int> subscribeToRoom(String roomId) {
    throw UnimplementedError("subscribeToRoom() has not been implemented.");
  }

  /// returns textureId of local video
  Future<int> startPublish() {
    throw UnimplementedError('startPublish() has not been implemented.');
  }
  Future<void> stopPublish() {
    throw UnimplementedError('stopPublish() has not been implemented.');
  }

  Future<void> startRecord() {
    throw UnimplementedError('startRecord() has not been implemented.');
  }

  Future<String> stopRecord() {
    throw UnimplementedError('stopRecord() has not been implemented.');
  }
}
