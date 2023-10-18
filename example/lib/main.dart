import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flowy_multimedia/flowy_multimedia.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';
  final _flowyMultimediaPlugin = FlowyMultimedia();
  bool _isRecording = false;

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    // We also handle the message potentially returning null.
    try {
      platformVersion = await _flowyMultimediaPlugin.getPlatformVersion() ??
          'Unknown platform version';
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    await _flowyMultimediaPlugin.subscribeToRoom('test');

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  void toggleRecording() {
    if (_isRecording) {
      _flowyMultimediaPlugin.stopRecord();
    } else {
      _flowyMultimediaPlugin.startRecord();
    }

    setState(() {
      _isRecording = !_isRecording;
    });
  }

  int textureId = 0;

  void subscribe() {
    _flowyMultimediaPlugin.subscribeToRoom("test").then((value) {
      setState(() {
        textureId = value;
      });
    });
  }

  void publish() {
    _flowyMultimediaPlugin.startPublish().then((value) {
      print("got texture id for local publish but not going to use yet: $value");
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
          child: Column(
            children: [
              Text('Running on: $_platformVersion\n'),
              // ElevatedButton(
              //     style: ElevatedButton.styleFrom(backgroundColor: Colors.red),
              //     onPressed: () async {
              //       toggleRecording();
              //     },
              //     child: Text(
              //         _isRecording ? 'Stop Recording' : 'Start Recording')),
              ElevatedButton(
                  onPressed: subscribe,
                  child: const Text("subscribe")),
              ElevatedButton(
                  onPressed: publish,
                  child: const Text("publish")),
              SizedBox(
                  height: 600,
                  width: 800,
                  child: Texture(
                    textureId: textureId,
                  ))
            ],
          ),
        ),
      ),
    );
  }
}
