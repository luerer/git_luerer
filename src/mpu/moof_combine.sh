apath="$HOME/Downloads/MPUFile/"
apath="$HOME/Videos/mmt/loveapartment_0203/audio"
from=10
to=100

offset=1037
dstdir=./audio
dstname=/tmp/test_audio.mp4
prefix="loveapartment_Audio1_MPU"


rm -rf $dstdir
mkdir -p $dstdir

for i in `seq 1 250`; do
    [ $i -le $from -o $i -gt $to ] && continue

    srcname=""
    if [ $i -lt 10 ]; then   
        srcname=$apath/${prefix}_000$i.mp4
    elif [ $i -lt 100 ]; then
        srcname=$apath/${prefix}_00$i.mp4
    elif [ $i -lt 250 ]; then
        srcname=$apath/${prefix}_0$i.mp4
    fi
    [ ! -f $srcname ] && continue
    ./cutfile $srcname $offset $dstdir/t$i.mp4 
done

cp -f $apath/${prefix}_00$from.mp4 $dstname
for i in `seq 1 250`; do
    [ ! -f $dstdir/t$i.mp4 ] && continue
    cat $dstdir/t$i.mp4 >> $dstname
done

exit 0
