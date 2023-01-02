import json
from time import sleep 
import pika



def json_payload(kpi,timestamp,value,ue_id,ue_tag=0,slice_id=None,source="RAN"):

    a = { "kpi": kpi,
         "slice_id": slice_id,
         "source": source,
         "timestamp": timestamp,
         "unit": "None",
         "value": value,
         "labels": [
             {"ue_id": ue_id},
             {"ue_tag":ue_tag}
             ] 
        }
    
    return json.dumps(a)


def main():
    #establising the connection with the broker
    connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
    channel = connection.channel()
    channel.queue_declare(queue='xapp-queue')


    
    # creating the json object for the KPI 
    a = json_payload(1,2,3,4,5,6)
    # sending the json to the broker 
    
    for i in range(5):
        sleep(5)
        print('message content is :',a)
        channel.basic_publish(exchange='',routing_key='xapp-queue',body=a)

        
    
    print('the sender sent the json data ')
    
    # closing the connection with the broker
    print("closing connection with the broker")
    connection.close()

if __name__ == "__main__":
    main()

    
    