<?php

class AddDamage extends BaseFile
{
    public function exec(&$assoc)
    {
        $url_files = "http://$_SERVER[SERVER_ADDR]/".Config::TARGET_DIR_FILES;

        $is_send_file = false;
        foreach ($_FILES as $name=>$file)
        {
            if ($name == "file" &&
                !empty($file["size"]))
            {
                $is_send_file = true;
                break;
            }
        }

        if (!$is_send_file ||
            !array_key_exists('id_complaint', $assoc))
        {
            Utils::printData(
                array('status' => Errors::NOT_SEND_FIELD,
                    'error' => Errors::instance()->data(Errors::NOT_SEND_FIELD),
                    'field' => "id_complaint",
                    'is_send_file' => $is_send_file));
        }

        $idComplaint = $assoc["id_complaint"];
        if (count($idComplaint) <= 0)
        {
            Utils::printData(
                array('status' => Errors::NOT_SEND_FIELD,
                    'error' => Errors::instance()->data(Errors::NOT_SEND_FIELD),
                    'field' => "id_complaint"));
        }

        $images = array();
        $file_url;
        $orig_file_name;
        foreach ($_FILES as $name=>$file)
        {
            if (strpos($name, "file") != 0 ||
                empty($file["size"]))
                continue;
            $orig_file_name = $file["name"];
            $file_name = $this->saveFile(Config::TARGET_DIR_FILES, $file);
            $file_url = "$url_files$file_name";

            array_push($images, $file);
            //echo "<p><a href=\"$url_files$file_name\">$url_files$file_name</a></p>";
        }

        $assoc["images"] = $images;
        $assoc["status"] = 1;
        $assoc["type_command"] = "save_damage_car";

        $result = NetworkManager::sendJson(Config::HOST_CPP, $assoc);
        if ($result == Errors::NOT_SEND_DATA_TO_REMORE_SERVER)
        {
            Utils::printData(
                array('status' => $result,
                    'error' => Errors::instance()->data($result)));
        }

        echo $result;
        exit();
    }
}

?>