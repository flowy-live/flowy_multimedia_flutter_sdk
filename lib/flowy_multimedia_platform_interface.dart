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

  /// initializes the current user with the media server
  Future<void> register() {
    throw UnimplementedError('register() has not been implemented.');
  }

  /// joins the room with the given conversationId for the
  /// media server to know to route media to the user
  Future<void> joinRoom(String conversationId) {
    throw UnimplementedError("joinRoom() has not been implemented.");
  }

  /// unsubscribe from getting media from anywhere
  Future<void> leave() {
    throw UnimplementedError('leave() has not been implemented.');
  }

  /// returns textureId
  Future<int> startReceiveVideo() {
    throw UnimplementedError('startReceiveVideo() has not been implemented.');
  }

  Future<void> stopReceiveVideo() {
    throw UnimplementedError('stopReceiveVideo() has not been implemented.');
  }

  Future<String?> startSendLiveAudio(String conversationId) {
    throw UnimplementedError('startSendLiveAudio() has not been implemented.');
  }

  Future<String?> stopSendLiveAudio() {
    throw UnimplementedError('stopSendLiveAudio() has not been implemented.');
  }

  Future<String?> startSendLiveVideo(String conversationId) {
    throw UnimplementedError('startSendLiveVideo() has not been implemented.');
  }

  Future<void> startRecord() {
    throw UnimplementedError('startRecord() has not been implemented.');
  }

  Future<String> stopRecord() {
    throw UnimplementedError('stopRecord() has not been implemented.');
  }
}
