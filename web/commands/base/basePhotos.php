<?php

class BasePhotos extends BaseCommand
{
    protected function get_name_file($id_driver, $pos)
    {
        $date = date_create();
        return date_format($date, 'Y-m-d_H-i-s-').substr((string)microtime(), 2, 7).'_'.$id_driver.'_'.$pos;
    }

    protected function saveImage($folder, &$imageInfo, $id_driver, $pos)
    {
        $file_name = $this->get_name_file($id_driver, $pos).substr($imageInfo["name"] , strrpos($imageInfo["name"], ".", -1));
        if (move_uploaded_file($imageInfo["tmp_name"], $_SERVER["DOCUMENT_ROOT"]."/$folder$file_name"))
        {
            return $file_name;
        }
        else
        {
            Utils::print_data(
                array('status' => Errors::NOT_SAVE_FILE, 
                'error' => Errors::instance()->data(Errors::NOT_SAVE_FILE)));
        }
    }
        
    protected function sendErrorToServer(&$assoc)
    {
        if(!array_key_exists('auto_id', $assoc))
            return;
        NetworkManager::sendHttp(Config::HOST_API, 
            array('type_query' => Config::TYPE_QUERY_API, 
                'name' => Config::LOGIN_API,
                'pass'=> Config::PASS_API,
                'auto_id'=>$assoc['auto_id'],
                'auto_status'=>18,
                'autoconfirm'=>1));
    }
        
    protected function sendOkToServer(&$assoc)
    {
        if(!array_key_exists('auto_id', $assoc))
            return;
        NetworkManager::sendHttp(Config::HOST_API, 
            array('type_query' => Config::TYPE_QUERY_API, 
            'name' => Config::LOGIN_API,
            'pass'=> Config::PASS_API,
            'auto_id'=>$assoc['auto_id'],
            'auto_status'=>18,
            'set_prev'=>1,
            'autoconfirm'=>1));
    }
        
    protected function selectCountImages()
    {
        $data = DBManager::selectData('SELECT COUNT(*) AS count_images FROM public.template_images', array());
        if(is_string($data))
        {
            Utils::print_data(
                array('status' => $data, 
                'error' => Errors::instance()->data($data)));
        }
        return $data[0]['count_images'];
    }
    
    protected function selectUrlPhotoDriver(&$assoc)
    {
        $data = DBManager::selectData(
                "SELECT DISTINCT ON(id_driver) id_driver, url_photo_driver, ".
                "to_char(info_photos.date_create, 'YYYY-MM-DD HH24:MI:SS.MS') AS date_create ".
                "FROM public.info_photos ".
                "WHERE (NOT url_photo_driver = '') AND id_driver = $1 ".
                "ORDER BY id_driver, info_photos.date_create DESC", 
                array($assoc['driver_id']));
        if(is_string($data))
        {
            Utils::print_data(
                array('status' => $data, 
                'error' => Errors::instance()->data($data)));
        }

        if(count($data) == 0)
            return '';
        
        return $data[0]['url_photo_driver'];
    }
        
    protected function checkOldPhotos(&$assoc)
    {
        $data = DBManager::selectData(
            'SELECT DISTINCT ON(id_driver) id_driver, status, '.
            'to_char(info_photos.date_create, \'YYYY-MM-DD HH24:MI:SS.MS\') AS date_create '.
            'FROM public.info_photos '.
            'WHERE id_driver = $1 '.
            'ORDER BY id_driver, info_photos.date_create DESC', 
            array($assoc['driver_id']));
        if(is_string($data))
        {
            Utils::print_data(
                array('status' => $data, 
                'error' => Errors::instance()->data($data)));
        }

        $status = $data[0]['status'];
        if($status == 1 ||
            $status == 3)  //если допущена или автодопущена
        {
            $this->sendErrorToServer($assoc);
            Utils::print_data(
                array('status' => Errors::NOT_SEND_SET_PHOTOS, 
                'error' => Errors::instance()->data(Errors::NOT_SEND_SET_PHOTOS)));
        }
    }
    
    protected function downloadPhotoDriver(&$assoc)
    {
        $tmp_file = NetworkManager::downloadFile(Config::getHostApiPhoto($assoc["id_company"]),
            array('act' => 'show', 
            'hash' => md5($assoc['driver_id'].'drivera'),
            'id'=> $assoc['driver_id'],
            'size'=>'wh',
            'type'=>'driver',
            'w'=>1024,
            'h'=>768));

        if (Errors::NOT_SEND_DATA_TO_REMORE_SERVER == $tmp_file)
            return '';
        
        $file_name = $this->get_name_file($assoc['driver_id'], 'driver');
//        $file_name = $this->get_name_file($assoc['driver_id'], 'driver').substr($imageInfo["name"] , strrpos($imageInfo["name"], ".", -1));
        if (file_put_contents($_SERVER["DOCUMENT_ROOT"]."/".Config::DRIVER_PHOTOS_DIR.$file_name, $tmp_file))
            return $file_name;
        
        return '';
    }

    protected function downloadCVDriver(&$assoc)
    {
        $tmp_file = NetworkManager::downloadFile(Config::getHostApiCV($assoc["id_company"]),
            array(
                'act' => 'show',
                'hash' => md5($assoc['driver_id'].'drivera'),
                'id' => $assoc['driver_id'],
                'size' => 'wh',
                'type' => 'driver',
                'w' => 1024,
                'h' => 768,
                'name' => Config::LOGIN_API,
                'pass' => Config::PASS_API));

        if (Errors::NOT_SEND_DATA_TO_REMORE_SERVER == $tmp_file)
            return '';

        $file_name = $this->get_name_file($assoc['driver_id'], 'driver-cv');
        if (file_put_contents($_SERVER["DOCUMENT_ROOT"]."/".Config::DRIVER_CV_DIR.$file_name, $tmp_file))
            return $file_name;

        return '';
    }
    
    public function exec(&$assoc)
    { }
}

?>