import cloudsight
import sys

auth = cloudsight.SimpleAuth('0XG_2yQHzZgMHyjVVVskNQ')
api = cloudsight.API(auth)
with open(sys.argv[1], 'rb') as f:
    response = api.image_request(f, sys.argv[1], {
        'image_request[locale]': 'en-US',
        })
status = api.wait(response['token'], timeout=30)
print str(status["name"])
