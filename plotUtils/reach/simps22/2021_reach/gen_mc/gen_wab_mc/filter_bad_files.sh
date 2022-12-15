
file=""
while IFS= read -r line; do
    if [[ $line == *wab* ]]
    then
        file=$line
        #path=realpath ${file}
    else
        count=$line
        if [[ $count -lt 10000 ]]
        then
            echo $count
            echo ${file}
            #echo $path
            mv $file ./low_event_count/ 
        fi
    fi
done < file_counts.txt
