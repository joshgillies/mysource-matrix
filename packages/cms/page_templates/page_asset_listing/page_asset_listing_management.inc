<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_asset_listing_management.inc,v 1.3.2.2 2004/02/21 12:27:59 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Asset_Listing_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Asset_Listing_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['types']		= Array(
										'added'   => '0.0.1',
										'type'    => 'serialise',
										'default' => Array(),
										);

		$this->vars['customised']	= Array(
										'added'   => '0.0.1',
										'type'    => 'serialise',
										'default' => Array(),
										);

		$this->vars['subs_only']	= Array(
										'added'     => '0.1',
										'type'      => 'boolean',
										'default'   => false,
										);

		$this->vars['sort_by']		= Array(
										'added'	=> '0.0.1',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												''				=> '-- No Sorting --',
												'assetid'		=> 'ID',
												'version'		=> 'Version',
												'name'			=> 'Name',
												'short_name'	=> 'Short Name',
												'created'		=> 'Created Date',
												'updated'		=> 'Last Updated Date',
											),
											'multiple' => false,
										),
										'default' => ''
										);

		$this->vars['group_by']		= Array(
										'added'	=> '0.0.1',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												'number'		=> 'Standard',
												'letter'		=> 'A-Z listing',
											),
											'multiple' => false,
										),
										'default' => 'number'
										);

		$this->vars['reverse_sort']	= Array(
										'added'     => '0.1',
										'type'      => 'boolean',
										'default'   => false,
										);

		$this->vars['num_per_page']	= Array(
										'added'   => '0.0.1',
										'type'    => 'int',
										'default' => 0,
										);

		$this->vars['prev_page_text']	= Array(
										'added'   => '0.0.1',
										'type'    => 'text',
										'default' => 'Previous'
										);

		$this->vars['next_page_text']	= Array(
										'added'   => '0.0.1',
										'type'    => 'text',
										'default' => 'Next'
										);
	

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return false;

		if ($current_version == '0.1') {
			// this upgrade changed the attribute 'types' to include a flag to
			// say if the type code should allow inheriting of its type descendants
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren(1, $this->getAssetType(), false);
			foreach ($children as $assetid => $type) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;

				$new_types = Array();
				$current_types = $asset->attr('types');
				foreach ($current_types as $type) $new_types[$type] = true;
				$asset->setAttrValue('types', $new_types);
				$asset->saveAttributes();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
		}//end version 0.1 upgrade

		return true;

	}//end upgrade()


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'				 => 'requestKeywords',
					'broadcast_type_code'		 => 'content_type_wysiwyg',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'side_of_link'		=> 'major',
						'indirect'			=> true,
						'exclusive'			=> null,
						'dependant'			=> 1,
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


}//end class

?>