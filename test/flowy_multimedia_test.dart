import 'package:flutter_test/flutter_test.dart';
import 'package:flowy_multimedia/flowy_multimedia.dart';
import 'package:flowy_multimedia/flowy_multimedia_platform_interface.dart';
import 'package:flowy_multimedia/flowy_multimedia_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockFlowyMultimediaPlatform
    with MockPlatformInterfaceMixin
    implements FlowyMultimediaPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final FlowyMultimediaPlatform initialPlatform = FlowyMultimediaPlatform.instance;

  test('$MethodChannelFlowyMultimedia is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelFlowyMultimedia>());
  });

  test('getPlatformVersion', () async {
    FlowyMultimedia flowyMultimediaPlugin = FlowyMultimedia();
    MockFlowyMultimediaPlatform fakePlatform = MockFlowyMultimediaPlatform();
    FlowyMultimediaPlatform.instance = fakePlatform;

    expect(await flowyMultimediaPlugin.getPlatformVersion(), '42');
  });
}
