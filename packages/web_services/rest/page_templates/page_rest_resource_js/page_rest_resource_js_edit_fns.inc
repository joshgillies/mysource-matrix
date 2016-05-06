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


        // enable the Ace editor to edit backend code (if available)
        if($o->isViperAceAvailable()) {
            // enable the Ace editor to edit backend code (if available)
            $o->enableViperAceEditor();
            $read_only = '';
            if(!$asset->writeAccess('attributes')) $read_only = 'sq-viper-ace-editor-textarea-readonly';

            text_area($prefix.'_javascript', $javascript, 100, 20, 0, 'style="" class="sq-parse-file-textarea sq-backend-code-editing sq-viper-ace-editor-textarea sq-viper-ace-editor-textarea-mode-js sq-viper-ace-editor-textarea-large '.$read_only.'"');
            return TRUE;
        }


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


	/**
    * Paint the interface for selecting JavaScript engine
    *
    * @param object &$asset the asset whose interface we are painting
    * @param object &$o     the outputter class
    * @param string $prefix prefix for the form elements
    *
    * @return boolean
    * @access public
    */
    function paintJavaScriptEngine(&$asset, &$o, $prefix)
    {
        $write_access = $asset->writeAccess('attributes');

		$options = Rest_Common::$supported_js_engines;
		$default_js_engine =  Rest_Common::$default_js_engine;

		$js_engine =  $asset->attr('js_engine');
		$js_engine_name = array_get_index($options, $js_engine, $options[$default_js_engine]);

		if ($write_access) {
			combo_box($prefix.'_js_engine', $options, FALSE, isset($options[$js_engine]) ? $js_engine : $default_js_engine);
		} else {
			echo $js_engine_name;
		}

		$o->note(translate('SpiderMokey requires engine binary path specified in External Tools Configuration.V8 engine requires v8js PECL extension to be installed.'));

		// Display warning note if the selected JS Engine is not available on the system
		if (!empty($js_engine) && !Rest_Common::jsEngineInstalled($js_engine)) {
			$o->note('<div class="sq-warning-message top-margin">'.sprintf(translate('<strong>Warning: </strong> %1$s is not installed on the system. JavaScript processing will not work.'), $js_engine_name).'</div>');
		}

		return $write_access;

	}//end paintJavaScriptEngine()


	/**
    * Processes the interface for selecting JavaScript engile
    *
    * @param object &$asset the asset to which we belong
    * @param object &$o     the outputter class
    * @param string $prefix prefix for the form element
    *
    * @return boolean
    * @access public
    */
    function processJavaScriptEngine(&$asset, &$o, $prefix)
    {
        // need to have write access to make any changes
        if (!$asset->writeAccess('attributes')) return FALSE;

		$options = Rest_Common::$supported_js_engines;
		$js_engine = isset($_POST[$prefix.'_js_engine']) ? $_POST[$prefix.'_js_engine'] : '';
		if (!empty($js_engine) && isset($options[$js_engine])) {
			return $asset->setAttrValue('js_engine', $js_engine);
		}

		return TRUE;

	}//end processJavaScriptEngine()


}//end class
?>
