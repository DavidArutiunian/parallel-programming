RESULT_PATH=result.txt
INPUT_PATH=Images\\FLAG_B24.BMP
OUTPUT_PATH=Images\\RESULT.BMP
CORES=$1
BLUR_RADIUS=124

if test -f "$RESULT_PATH"; then
    rm $RESULT_PATH
fi

touch $RESULT_PATH

for THREADS in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
do
    echo "Running MultiThreadBlur_x32.exe with $THREADS threads on $CORES cores"

    ./MultiThreadBlur_x32.exe $INPUT_PATH $OUTPUT_PATH $THREADS $CORES $BLUR_RADIUS | tail -1 >> $RESULT_PATH
done
