import datetime
import random
import json

import matplotlib.animation as animation
import matplotlib.pyplot as plt
import paho.mqtt.client as mqtt

# MUDE esses parametros se mudar o teste
topic = 'kilos/#'
log_name = 'kilos.log'

# use ggplot style for more sophisticated visuals
plt.style.use('ggplot')

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs, xs1 = [], []
ys, ys1 = [], []

# load log
try:
    log = open(log_name, 'r')
    log_content = log.read()
    log.close()

    for r in log_content.split('\n'):
        dt, v1, v2 = r.split(',')
        xs.append(dt)
        ys.append(int(v1))
        xs1.append(dt)
        ys1.append(int(v2))
except:
    pass


# This function is called periodically from FuncAnimation
def animate(i, xs, ys, xs1, ys2):
    # Draw x and y lists
    ax.clear()
    ax.plot(xs, ys)
    ax.plot(xs1, ys1)

    # Format plot
    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title('Carga ao longo do tempo')
    plt.ylabel('Carga (bit)')


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        client.subscribe(topic)

def on_message(client, user_data, msg):
    payload = json.loads(msg.payload)

    print(payload)
    dt = datetime.datetime.now().strftime('%H:%M:%S')

    xs.append(dt)
    ys.append(int(payload['value1']))
    xs1.append(dt)
    ys1.append(int(payload['value2']))    

    log = open(log_name, 'a+')
    log.write('{},{},{}\n'.format(dt, int(payload['value1']), int(payload['value2'])))
    log.close()


if __name__ == '__main__':
    client = mqtt.Client()

    client.on_connect = on_connect
    client.on_message = on_message

    client.connect('mqtt.positiva.app')
    client.loop_start()

    ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys, xs1, ys1), interval=10000)
    plt.show()

