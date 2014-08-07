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
* $Id: content_type_image_management.inc,v 1.8 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Image_Management
*
* @see Asset_Management
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Image_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'display_settings'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'on_click'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'none',
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'none'				=> 'No Action',
																						'new_window'		=> 'Open Image in New Window',
																						'current_window'	=> 'Open Image in Current Window',
																						'inline'			=> 'Open Inline Image Preview',
																					   ),
																   ),
											   ),
					  );

	}//end constructor


	/**
	* Upgrade script
	*
	* @param string	$current_version	the version we are upgrading FROM
	*
	* @return boolean
	* @access private
	* @see Asset_Management::_upgrade()
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version, '0.2', '<')) {
			pre_echo('UPGRADING FILE ASSETS - FROM VERSION '.$current_version);
			$preferences = Array();
			if (is_file(SQ_DATA_PATH.'/private/conf/preferences.inc')) {
				include SQ_DATA_PATH.'/private/conf/preferences.inc';

				$type_code = $this->getAssetType();
				$preferences[$type_code]['SQ_CONTENT_TYPE_IMAGE_DEFAULTS']['default']['html_layout'] = 'table';
				$preferences[$type_code]['SQ_CONTENT_TYPE_IMAGE_FIELDS']['default']['html_layout'] = TRUE;
				$preferences[$type_code]['SQ_CONTENT_TYPE_IMAGE_CLASSES']['default']['image_div'] = 'image_div';
				$preferences[$type_code]['SQ_CONTENT_TYPE_IMAGE_CLASSES']['default']['caption_div'] = 'caption_div';
				$preferences[$type_code]['SQ_CONTENT_TYPE_IMAGE_PREVIEW']['default']['image_div'] = 'preview_image_div';
				$preferences[$type_code]['SQ_CONTENT_TYPE_IMAGE_PREVIEW']['default']['close_preview_div'] = 'close_preview_div';

				// save the updated preferences file
				$str = '<'.'?php $preferences = '.var_export($preferences, TRUE).'; ?'.'>';
				if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/preferences.inc')) {
					return FALSE;
				}
			}
		}//end if version 0.2

		return TRUE;

	}//end _upgrade()


}//end class

?>
