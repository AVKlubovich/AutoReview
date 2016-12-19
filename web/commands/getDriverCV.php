<?php

class GetDriverCV extends BasePhotos
{
    public function exec(&$assoc)
    {
        $url_drivers_cvs = "http://$_SERVER[SERVER_ADDR]/".Config::DRIVER_CV_DIR;

        $tmp = $this->downloadCVDriver($assoc);
        if (!empty($tmp))
        {
            $url_cv_driver = "$url_drivers_cvs$tmp";
            $assoc["url_cv_driver"] = $url_cv_driver;
        }

        $assoc["count"] = count($tmp);

        echo Utils::toJson($assoc);
        exit();
    }
}

?>