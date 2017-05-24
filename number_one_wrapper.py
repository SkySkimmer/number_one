#!/usr/bin/python3

import sys, json, struct, subprocess

# Read a message from stdin and decode it.
def getMessage():
  rawLength = sys.stdin.buffer.read(4)
  if len(rawLength) == 0:
      sys.exit(0)
  messageLength = struct.unpack('@I', rawLength)[0]
  message = sys.stdin.buffer.read(messageLength).decode('utf-8')
  return json.loads(message).encode('utf-8')

# Encode a message for transmission, given its content.
def encodeMessage(messageContent):
  encodedContent = json.dumps(messageContent)
  encodedLength = struct.pack('@I', len(encodedContent))
  return {'length': encodedLength, 'content': encodedContent}

# Send an encoded message to stdout.
def sendMessage(encodedMessage):
  sys.stdout.buffer.write(encodedMessage['length'])
  sys.stdout.buffer.write(encodedMessage['content'].encode('utf-8'))
  sys.stdout.flush()

received = getMessage()
runner = subprocess.run(["/usr/local/bin/number_one_main"], stdout=subprocess.PIPE, input=received)
sendMessage(encodeMessage(runner.stdout.decode('utf-8')))
