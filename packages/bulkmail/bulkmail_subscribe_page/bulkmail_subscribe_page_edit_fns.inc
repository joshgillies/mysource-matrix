<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: bulkmail_subscribe_page_edit_fns.inc,v 1.5 2012/08/30 00:57:13 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_user_group_manager/page_user_group_manager_edit_fns.inc';

/**
* Bulkmail_Subscribe_Page_Edit_Fns
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Subscribe_Page_Edit_Fns extends Page_User_Group_Manager_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor

    /**
    * Paints the interface for the CAPTCHA Font field
    *
    * @param object &$asset the asset to which we belong
    * @param object &$o     the outputter class
    * @param string $prefix prefix for the form element
    *
    * @return boolean
    * @access public
    */
    function paintCaptchaFontField(&$asset, &$o, $prefix)
    {
        if ($asset->writeAccess('attributes')) {
            asset_finder($prefix.'_captcha_ttf', $asset->attr('captcha_ttf'), Array('file' => 'D'));
        } else {
            $asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset->attr('captcha_ttf'));
            $ttf_file_id = $asset->attr('captcha_ttf');
            if ($ttf_file_id > 0) {
                echo get_asset_tag_line($ttf_file_id);
            } else {
                echo translate('None selected');

            }
        }
        return TRUE;

    }//end paintCaptchaFontField()


    /**
    * Processes the interface for the CAPTCHA Font field
    *
    * @param object &$asset the asset to which we belong
    * @param object &$o     the outputter class
    * @param string $prefix prefix for the form element
    *
    * @return boolean
    * @access public
    */
    function processCaptchaFontField(&$asset, &$o, $prefix)
    {
        if (isset($_POST[$prefix.'_captcha_ttf'])) {
            $ttf_file_id = $_POST[$prefix.'_captcha_ttf']['assetid'];

            if ($ttf_file_id != $asset->attr('captcha_ttf')) {
                if ($ttf_file_id > 0) {
                    // Grab the specified asset - it should be a File - and ensure that it is a TrueType Font
                    $ttf_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($ttf_file_id);

                    $ttf_file_info = $ttf_file->getExistingFile();
                    $filename = strtolower($ttf_file_info['filename']);

                    $extension_index = strpos($filename, '.ttf');
                    if (($extension_index !== FALSE) && ($extension_index == strlen($filename)-4)) {
                        return $asset->setAttrValue('captcha_ttf', $ttf_file_id);
                    } else {
                        trigger_localised_error('CMS0109', translate('The CAPTCHA Font specified was not a TrueType Font File (ie; did not have a .ttf extension)'), E_USER_WARNING);
                    }
                } else {
                    return $asset->setAttrValue('captcha_ttf', NULL);
                }
            }
        }

        return FALSE;

    }//end processCaptchaFontField()
    
}//end class

?>
