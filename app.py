from flask import Flask, request
import requests
from flask_mysqldb import MySQL
import yaml

app = Flask(__name__)

db = yaml.load(open('db.yaml'))
app.config['MYSQL_HOST'] = db['mysql_host']
app.config['MYSQL_USER'] = db['mysql_user']
app.config['MYSQL_PASSWORD'] = db['mysql_password']
app.config['MYSQL_DB'] = db['mysql_db']

mysql = MySQL(app)


@app.route('/', methods=['POST'])
def index():
    json_data = request.json
    water1 = json_data["value"]
    print(water1)
    wc = 1
    cur = mysql.connection.cursor()
    cur.execute("INSERT INTO database1(water, verstopt) VALUES(%s, %s)", (water1, wc,))
    mysql.connection.commit()
    cur.close()
    return 'succes'


@app.before_request
def log_request_info():
    app.logger.debug('Headers: %s', request.headers)
    app.logger.debug('Body: %s', request.get_data())


if __name__ == "__main__":
    app.run(debug=True, port=5000, host='192.168.1.53')

