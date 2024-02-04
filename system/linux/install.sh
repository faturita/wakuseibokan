sudo mkdir /usr/games/waku
sudo mv waku.tgz /usr/games/waku
cd /usr/games/waku
sudo tar xvzf waku.tgz
sudo cp dependencies/*.lib /usr/local/lib
sudo cp waku.desktop /usr/share/applications/waku.desktop
sudo cp images/waku.png /usr/games/waku/waku.png
chmod +x waku



