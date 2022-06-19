from multiprocessing import Queue, Process
from web_control import main

moving_queue = Queue()
speed_queue = Queue()

from flask import Flask, render_template, request

app = Flask(__name__)


@app.route('/')
def hello_world():
    return render_template("index.html")


@app.route('/button/<button_name>', methods=["GET"])
def button(button_name):
    print("press: ", button_name)
    moving_queue.put(button_name)
    return ""


@app.route('/input_range/<param_name>', methods=["GET"])
def input_range(param_name):
    current_value = float(request.args["current_value"])
    print("speed: ", current_value)
    speed_queue.put(current_value)
    return ""


if __name__ == '__main__':
    control_loop = Process(target=main, args=(moving_queue, speed_queue))
    control_loop.start()
    app.run(host="0.0.0.0", threaded=True)
