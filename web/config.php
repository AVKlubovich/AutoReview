<?php

require_once 'errors.php';
require_once 'utils.php';


class Config
{
    const TARGET_DIR_FILES = "okk/files/";
    const DRIVER_PHOTOS_DIR = "okk/drivers/";
    const DRIVER_CV_DIR = "okk/drivers_cv/";

    const HOST_DB = "192.168.202.222";
    const PORT_DB = 5432;
    const NAME_DB = "photo_validator_db";
    const USER_DB = "postgres";
    const PASS_DB = "11";

    const HOST_API_PHOTO = "http://192.168.211.30:81/api/api_general_gruz_msk_img.php";
    const HOST_API_CV = "http://192.168.211.30:81/api/api_l_gruz_msk.php";
    const LOGIN_API = "okk";
    const PASS_API = "ufhe_e7bbbewGhed16t";

    const HOST_CPP = "http://192.168.213.142:81/";

    public static function getHostApiPhoto($companyId)
    {
        $HOSTS_API_PHOTO = array(
            1  => "http://192.168.211.30:81/api/api_general_gruz_msk_img.php",
            2  => "http://192.168.211.30:81/api/api_general_gruz_spb_img.php",
            3  => "http://192.168.211.30:81/api/api_general_taxi_spb_img.php",
            4  => "http://192.168.211.30:81/api/api_general_gruz_ros_img.php",
            5  => "http://192.168.211.30:81/api/api_general_gruz_kda_img.php",
            6  => "http://192.168.211.30:81/api/api_general_gruz_kgd_img.php",
            7  => "http://192.168.211.30:81/api/api_general_gruz_vor_img.php",
            8  => "http://192.168.211.30:81/api/api_general_gruz_ufa_img.php",
            9  => "http://192.168.211.30:81/api/api_general_gruz_per_img.php",
            10 => "http://192.168.211.30:81/api/api_general_gruz_kzn_img.php",
            11 => "http://192.168.211.30:81/api/api_general_gruz_ufa_img.php"
        );

        if (array_key_exists($companyId, $HOSTS_API_PHOTO))
        {
            return $HOSTS_API_PHOTO[$companyId];
        }
        else
        {
            Utils::printData(
                array('status' => Errors::COMPANY_URL_NOT_FOUND,
                    'error' => Errors::instance()->data(Errors::COMPANY_URL_NOT_FOUND)
                ));
        }
    }

    public static function getHostApiCV($companyId)
    {
        $HOSTS_API_CV = array(
            1  => "http://192.168.211.30:81/api/api_l_gruz_msk_img.php",
            2  => "http://192.168.211.30:81/api/api_l_gruz_spb_img.php",
            3  => "http://192.168.211.30:81/api/api_l_taxi_spb_img.php",
            4  => "http://192.168.211.30:81/api/api_l_gruz_ros_img.php",
            5  => "http://192.168.211.30:81/api/api_l_gruz_kda_img.php",
            6  => "http://192.168.211.30:81/api/api_l_gruz_kgd_img.php",
            7  => "http://192.168.211.30:81/api/api_l_gruz_vor_img.php",
            8  => "http://192.168.211.30:81/api/api_l_gruz_ufa_img.php",
            9  => "http://192.168.211.30:81/api/api_l_gruz_per_img.php",
            10 => "http://192.168.211.30:81/api/api_l_gruz_kzn_img.php",
            11 => "http://192.168.211.30:81/api/api_l_gruz_ufa_img.php"
        );

        if (array_key_exists($companyId, $HOSTS_API_CV))
        {
            return $HOSTS_API_CV[$companyId];
        }
        else
        {
            Utils::printData(
                array('status' => Errors::COMPANY_URL_NOT_FOUND,
                    'error' => Errors::instance()->data(Errors::COMPANY_URL_NOT_FOUND)
                ));
        }
    }
}

?>