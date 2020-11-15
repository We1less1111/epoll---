temp=$(ifconfig|grep broadcast|awk '{print $2}')
sed -i "s/[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}/${temp}/g" myclient.c
gcc -o myclient myclient.c -lpthread
kill -9 $(lsof -i:6666|awk '{print $2}')
kill -9 $(ps -ef|grep myserver|awk '{print $3}')
echo "new ip=${temp},pid=${pid}"
echo "gcc -o myclient.c myclient success"
