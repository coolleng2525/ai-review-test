#!/bin/bash

cmm_start_container(){
local fn_name="$1"
local name="$2"
local type="$3"
local n=3
echo "start container:$name"
idall=`docker ps -a| grep ${name} | awk '{print($1)}'`
idrun=`docker ps | grep ${name} | awk '{print($1)}'`
if [ -n "$idrun" ];then
        if [ "$type" == "force" ];then
                echo "stop and remove  ${idrun} existed container"
                docker stop ${idrun}
                docker rm ${idrun}
                sleep 1
        else
                echo "Keep it attach container"
                docker attach ${idrun}
                return
        fi
elif [ -n "$idall" ];then
        if [ "$type" == "force" ];then
                echo "stop and remove  ${idall} existed container"
                docker rm ${idall}
                sleep 1
        else
                echo "Keep it existed container"
                docker start ${idall}
                docker attach ${idall}
                return
        fi
fi
echo "start $name contain by $fn_name "
$fn_name "$name"
n=1
while [ $n -le 3 ]; do 
    id=`docker ps | grep ${name} | awk '{print($1)}'`
    if [ -z "$id" ];then
        echo "wait 1 second, and get th id."
        sleep 1
        id=`docker ps | grep ${name} | awk '{print($1)}'`
    else
        break
    fi
n=$(( n+1 ));
done

echo "echo id: $id"
if [ -n "$id" ];then
    docker attach ${id}
fi

}

stop_container(){
    local name=$1
    ids=`docker ps -a | grep ${name} | awk '{print($1)}'`
    for id in $ids
    do
        if [ -n "$id" ];then
            docker stop $id
        fi
    done
}
clean_container(){
    echo "clean container"
}

remove_container(){
    local name=$1
    if [ -n $name ];then
        ids=`docker ps -a | grep ${name} | awk '{print($1)}'`
        for id in $ids
        do
            if [ -n "$id" ];then
                docker stop $id
                docker rm $id
            fi
        done
    else 
        clean_container
    fi
}


attach_container(){
    local name=$1
    echo ""
    id=`docker ps | grep ${name} | awk '{print($1)}'`
    idall=`docker ps -a | grep ${name} | awk '{print($1)}'`
    if [ -n "$id" ];then
        docker attach "$id"
    elif [ -n "$idall" ];then
            docker start ${idall}
            docker attach "$id"
    else
        echo "$name container is not existed"
    fi
}

show_container(){
    local name=$1
    if [ -n $name ];then
    docker ps -a | grep "$name"
    else
    docker ps -a
    fi
}

show_running_container(){
    local name=$1
    if [ -n $name ];then
    docker ps  | grep "$name"
    else
    docker ps
    fi
}
clean_images(){
    echo "clean imgaes"
    local line=""
    local repo=""
    local tag=""
    declare -i head=0
    docker images > /tmp/comm_docker_text.txt
    for line in $(ls -a | sort); do echo $line; done
    while read line < /tmp/comm_docker_text.txt; do
    # for line in $(docker images); do
        echo "$line"
        if [ $head -eq 0 ];then
            head=1
            continue
        else
            repo=`echo $line |  awk '{ print $1}'`
            tag=`echo $line |  awk '{ print $2}'`
            echo "repo:(${repo}) tag:(${tag})"
        fi
    done
}
show_images(){
    local name=$1
    if [ -n $name ];then
    docker images | grep "$name"
    else
    docker images
    fi
}
remove_images(){
    local name=$1
    if [ -n "$name" ];then
        docker rmi "$name"
    else
        clean_images
    fi
}

is_existed_image(){
    local image_name="$1"
    local real_image=${IMAGE%:*}
    local tag_image=${IMAGE#*:}
    # | awk '{print $1}'
    local all_ruckus_images="`docker images | grep ^ruckus `"

    # echo "$image_name" | grep "^ruckus"  2>/dev/null >/dev/null
    # local ret=$?
    if [ -z "$all_ruckus_images" ];then
        all_ruckus_images=$image_name
    fi

    if [ -n "$tag_image" ];then
        echo "$all_ruckus_images" | grep ^$real_image | grep "$tag_image" 2>/dev/null >/dev/null
    else
        echo "$all_ruckus_images" | grep ^$real_image 2>/dev/null >/dev/null
    fi
    return $?
}

function show_all_logs_size
{
    docker ps -aq | xargs -I '{}' docker inspect --format='{{.LogPath}}' '{}' | xargs ls -lh
}

function show_logs_size
{
     local name=$1
     docker inspect --format='{{.LogPath}}' "$name" | xargs ls -lh
}

show_title(){
    local type="$1"
    local content="$2"
    if [ "$type" = "start" ];then
        echo ""
    fi
echo -e "
********************************************************************
$type $content
********************************************************************
"
if [ "$type" = "end" ];then
        echo ""
fi
}

show_comm_usage(){
echo "
comm Version 1.0
comm usage: 
        $0 -s|--show                    ;show all container
        $0 -r|--running                 ;show running container
        $0 -R|-rm|--remove  name        ;remove container
        $0 -si                          ;show all images
        $0 -Ri  name                    ;remove images
        $0 -S|--stop        name        ;stop container.
        $0 -a|--attach      name        ;start container.

Ex 1:   $0 -s
Ex 2:   $0 -r
Ex 3:   $0 -R name
Ex 4:   $0 -S name
Ex 4:   $0 -a name
"
}

cmd=$1
name=$2
if [ "remove" = "$cmd" ] || [ "-rm" = "$cmd" ] || [ "-R" = "$cmd" ];then
    remove_container $name
    exit
elif [ "--stop" = "$cmd" ] || [ "-S" = "$cmd" ];then
    stop_container $name
    exit
elif [ "--attach" = "$cmd" ] || [ "-a" = "$cmd" ];then
    attach_container $name
    exit    
elif [ "--show" = "$cmd" ] || [ "-s" = "$cmd" ];then
    show_container "$name"
    exit
elif [ "--running" = "$cmd" ] || [ "-r" = "$cmd" ];then
    show_running_container "$name"
    exit
elif [ "-si" = "$cmd" ];then
    show_images
    exit
elif [ "-Ri" = "$cmd" ];then
    remove_images "$name"
    exit
elif [ "-hc" = "$cmd" ];then
    show_comm_usage
    exit
elif [ "-logall" = "$cmd" ];then
    show_all_logs_size
      exit
fi    