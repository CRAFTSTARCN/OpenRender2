from concurrent.futures import ThreadPoolExecutor, wait, ALL_COMPLETED

WorkingThread = ThreadPoolExecutor(max_workers=14)

works = []

def add_work(fn, *args, **kwargs):
    task = WorkingThread.submit(fn, *args, **kwargs)
    works.append(task)


def all_shutdown():
    wait(fs=works, timeout=None, return_when=ALL_COMPLETED)
    WorkingThread.shutdown()
