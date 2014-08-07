<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: page_rest_resource_js_edit_fns.inc,v 1.3 2010/05/05 07:16:13 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource_edit_fns.inc';


/**
* Page_REST_Resource_JS_Edit_Fns
*
* Purpose
*
*
* @author  James Hurst <jhurstÂ@squiz.co.uk>
* @version $Revision: 1.3 $
* @package Squiz_UK_Clients
* @subpackage rest_resource_js
*/
class Page_REST_Resource_JS_Edit_Fns extends Page_REST_Resource_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


  	/**
    * Paint the interface for selecting JavaScript include files
    *
    * @param object &$asset the asset whose interface we are painting
    * @param object &$o     the outputter class
    * @param string $prefix prefix for the form elements
    *
    * @return boolean
    * @access public
    */
    function paintJavaScriptFiles(&$asset, &$o, $prefix)
    {
        $write_access = $asset->writeAccess('attributes');
        $javascript_files = $asset->attr('javascript_files');

        if ($write_access) {
            require_once SQ_LIB_PATH.'/html_form/html_form.inc';
            multiple_asset_finder($prefix.'_javascript_files', $javascript_files, Array('file' => 'D'));
        } else {
                echo '<ul>';
                foreach ($javascript_files as $index => $assetid) {
                        echo '<li>'.get_asset_tag_line($assetid).'</li>';
                }
                echo '</ul>';
        }

        return $write_access;

    }//end paintJavaScriptFiles()


    /**
    * Processes the interface for selecting JavaScript files.
    *
    * @param object &$asset the asset to which we belong
    * @param object &$o     the outputter class
    * @param string $prefix prefix for the form element
    *
    * @return boolean
    * @access public
    */
    function processJavaScriptFiles(&$asset, &$o, $prefix)
    {
        // need to have write access to make any changes
        if (!$asset->writeAccess('attributes')) return FALSE;
        if (empty($_POST[$prefix.'_javascript_files'])) return FALSE;

        $javascript_files = $_POST[$prefix.'_javascript_files'];
        $save_javascript_files = Array();
        foreach ($javascript_files as $index => $asset_info) {
            if (!empty($asset_info['assetid'])) {
                $save_javascript_files[] = $asset_info['assetid'];
            }//end if
        }//end foreach

        return $asset->setAttrValue('javascript_files', $save_javascript_files);

    }//end processJavaScriptFiles()
	
	
	/**
    * Paint the interface for selecting JavaScript include files
    *
    * @param object &$asset the asset whose interface we are painting
    * @param object &$o     the outputter class
    * @param string $prefix prefix for the form elements
    *
    * @return boolean
    * @access public
    */
    function paintJavaScript(&$asset, &$o, $prefix)
    {
        $write_access = $asset->writeAccess('attributes');
        $javascript = $asset->attr('javascript');

        if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			text_area($prefix.'_javascript', $javascript, 100, 20, 0, 'style="font-family: monospace;');
		} else {
			echo "<pre>" . htmlspecialchars($javascript) . "</pre>\n";
		}

        return $write_access;

    }//end paintJavaScript()


    /**
    * Processes the interface for selecting JavaScript files.
    *
    * @param object &$asset the asset to which we belong
    * @param object &$o     the outputter class
    * @param string $prefix prefix for the form element
    *
    * @return boolean
    * @access public
    */
    function processJavaScript(&$asset, &$o, $prefix)
    {
        // need to have write access to make any changes
        if (!$asset->writeAccess('attributes')) return FALSE;

		$js = isset($_POST[$prefix.'_javascript']) ? $_POST[$prefix.'_javascript'] : '';
		return $asset->setAttrValue('javascript', $js);

    }//end processJavaScript()

}//end class
?>
