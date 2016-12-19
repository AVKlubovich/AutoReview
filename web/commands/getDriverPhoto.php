<?php

class GetDriverPhoto extends BasePhotos
{
    public function exec(&$assoc)
    {
        $url_drivers_photos = "http://$_SERVER[SERVER_ADDR]/".Config::DRIVER_PHOTOS_DIR;

        $tmp = $this->downloadPhotoDriver($assoc);
        if (!empty($tmp))
        {
            $url_photo_driver = "$url_drivers_photos$tmp";
            $assoc["url_photo_driver"] = $url_photo_driver;
        }

        $assoc["count"] = count($tmp);

        echo Utils::toJson($assoc);
        exit();
    }
}

?>