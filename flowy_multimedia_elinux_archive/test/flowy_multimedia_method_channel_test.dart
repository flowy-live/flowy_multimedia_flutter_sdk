import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:flowy_multimedia/flowy_multimedia_method_channel.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  MethodChannelFlowyMultimedia platform = MethodChannelFlowyMultimedia();
  const MethodChannel channel = MethodChannel('flowy_multimedia');

  setUp(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger.setMockMethodCallHandler(
      channel,
      (MethodCall methodCall) async {
        return '42';
      },
    );
  });

  tearDown(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger.setMockMethodCallHandler(channel, null);
  });

  test('getPlatformVersion', () async {
    expect(await platform.getPlatformVersion(), '42');
  });
}
