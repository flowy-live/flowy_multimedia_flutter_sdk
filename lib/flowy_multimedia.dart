// You have generated a new plugin project without specifying the `--platforms`
// flag. A plugin project with no platform support was generated. To add a
// platform, run `flutter create -t plugin --platforms <platforms> .` under the
// same directory. You can also find a detailed instruction on how to add
// platforms in the `pubspec.yaml` at
// https://flutter.dev/docs/development/packages-and-plugins/developing-packages#plugin-platforms.

import 'flowy_multimedia_platform_interface.dart';

class FlowyMultimedia {
  FlowyMultimedia();

  int? textureId;

  bool get isInitialized => textureId != null;

  Future<String?> getPlatformVersion() {
    return FlowyMultimediaPlatform.instance.getPlatformVersion();
  }

  Future<int> subscribeToRoom(String roomId) async {
    int textureId = await FlowyMultimediaPlatform.instance.subscribeToRoom(roomId);

    print('textureId received from platform for remote video: $textureId');

    return textureId;
  }

  /// unsubscribe from getting media from anywhere
  void leave() {
  }

  Future<int> startPublish() async {
    return await FlowyMultimediaPlatform.instance.startPublish();
  }

  Future<void> stopPublish() {
    return FlowyMultimediaPlatform.instance.stopPublish();
  }

  Future<void> startRecord() {
    return FlowyMultimediaPlatform.instance.startRecord();
  }

  Future<String> stopRecord() {
    return FlowyMultimediaPlatform.instance.stopRecord();
  }
}
