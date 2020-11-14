#!/usr/bin/env python
import os
import json
from threading import Lock
from flask import Flask, render_template, session, request
from flask_socketio import SocketIO, emit, disconnect

from flask import Flask
app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode=None, ping_interval=3000, ping_timeout=6000)

#@app.route('/')
#def hello_world():
#    return 'Hello, World!'

@app.route('/')
def index():
    return render_template('index.html', async_mode=socketio.async_mode)

# =================================
#    Socket communication example
# =================================
thread = None
thread_lock = Lock()
max_iteration= 100
boardList = {}
def background_thread():
    """Example of how to send server generated events to clients."""
    count = 0
    while True:
        socketio.sleep(10)
        count += 1
        #socketio.emit('my_response',
        #              {'data': 'Server generated event', 'count': count},
        #              namespace='/test')

@socketio.on('connect', namespace='/test')
def test_connect():
    global thread
    with thread_lock:
        if thread is None:
            thread = socketio.start_background_task(background_thread)

@socketio.on('disconnect', namespace='/test')
def test_disconnect():
    if request.sid in boardList:
        del boardList[request.sid]
    print('Client disconnected', request.sid)
    print(len(boardList), ' clients left')

@socketio.on('my_event', namespace='/test')
def test_message(message):
    print("Hello", message['data'])
    session['receive_count'] = session.get('receive_count', 0) + 1
    emit('my_response',
         {'data': message['data'], 'count': session['receive_count']})

# ==============================================
#    Build and communication with AMOBA kernel
# ==============================================
from subprocess import Popen, PIPE, STDOUT
p = None
@socketio.on('build', namespace='/test')
def build(message):
    global p
    print("Build")
    os.system("pwd")
    os.system("ls")
    p = Popen(['./bins/AMOBA_4x{}'.format(message["COL"]), '--test', "--quiet"],
            stdout=PIPE, stdin=PIPE, stderr=PIPE, encoding='utf8', bufsize=1, universal_newlines=True)
    os.system("ls")
    os.system("pwd")

    # === Get current board with no action ===
    move({"row":"-1", "col":"-1"})

@socketio.on('move', namespace='/test')
def move(message):
    if(message["row"] != "-1"):
        print("Move {} {}".format(message["row"], message["col"]))
        p.stdin.write("{} {}\n".format(message["row"], message["col"]))
    else:
        print("Init move")
    
    moves = {"white":[], "black":[]}
    line = ""
    end = "false"
    while(line != "[RES]\n"):
        line = p.stdout.readline()
        #print(line[:6])
        if(line[:6] == "[DIFF]"):
            whites = p.stdout.readline()[:-2].split(' ')
            blacks = p.stdout.readline()[:-2].split(' ')
            moves["white"].append([w for w in whites])
            moves["black"].append([b for b in blacks])
        elif(line[:5] == "[END]"):
            p.terminate()
            print("Game over")
            end = "true"
            break
    
    print("Sending moves: ", json.dumps(moves))
    emit('update_nodes',{'data': json.dumps(moves), "end":end})

if __name__ == '__main__':
    socketio.run(app, debug=True, port= 5000)