from flask import Flask, request
import random
import uuid
import queue

app = Flask(__name__) 

WORK_SIZE = 100_000

#"work" refers to a range of seeds which are being checked by a single client.
class Work:
    def __init__(self, start, end, wid=0):
        self.wid = wid
        self.start = start
        self.end = end
        self.complete = False

    def as_json(self): 
        return {
            "wid": self.wid,
            "start_seed": self.start,
            "end_seed": self.end
        }

    def __repr__(self):
        return f"(Work({self.wid}): {self.start}:{self.end})"

projects = {
    0: {
        "start_seed": 0,
        "total_seed_space": 1_000_000,
        "imcomplete_work": queue.Queue(),
        "in_progress_work": [],
        "complete_work": [],
        "clients": []
    }
}

clients = {
    
}

@app.route("/establish_connection")
def establish_connection():
    cid = random.randrange(0, 1000000)
    
    clients[cid] = {
        "project": 0
    }

    print(clients)
    projects[0]["clients"].append(cid)

    return {
        "cid": cid
    }

@app.route("/get_work/<cid>")
def get_work(cid):
    pid = clients.get(int(cid)).get("project")

    work_block = projects[0]["imcomplete_work"].get()
    projects[0]["in_progress_work"].append(work_block)

    return {
        "work_block": work_block.as_json()
    }

if __name__ == "__main__":
    start_seed = projects[0]["start_seed"]
    end_seed = start_seed + projects[0]["total_seed_space"]

    work_blocks = [Work(start, start + WORK_SIZE, wid=wid) for wid, start in enumerate(range(start_seed, end_seed, WORK_SIZE))]
    for block in work_blocks:
        projects[0]["imcomplete_work"].put(block)

    # print(projects[0]["incomplete_work"])
    # exit()

    app.run(debug=True)