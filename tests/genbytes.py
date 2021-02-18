f = open('test.raw', 'wb')

for i in range(10):
    for j in range(1024):
        f.write(str.encode(f"{i}"));
