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
* $Id: page_asset_listing_management.inc,v 1.23 2005/02/13 23:58:31 tbarrett Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Asset_Listing_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.23 $
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
										'added'   => '0.1',
										'type'    => 'serialise',
										'default' => Array(),
										);

		$this->vars['subs_only']	= Array(
										'added'     => '0.1',
										'type'      => 'boolean',
										'default'   => false,
										'parameters' => Array(
															'allow_empty' => false
														),
										);

		$this->vars['direction']	= Array(
										'added'	=> '0.6',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												'up'	=> 'Up the tree',
												'down'	=> 'Down the tree',
											),
											'multiple' => false,
											'allow_empty' => false,
										),
										'default' => 'down'
										);

		$this->vars['asset_positions']	= Array(
										'added'   => '0.9',
										'type'    => 'serialise',
										'default' => Array(),
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
												'published'		=> 'Last Published Date',
											),
											'multiple' => false,
											'allow_empty' => false,
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
												'random'		=> 'Random',
											),
											'multiple' => false,
											'allow_empty' => false,
										),
										'default' => 'number'
										);

		$this->vars['reverse_sort']	= Array(
										'added'     => '0.1',
										'type'      => 'boolean',
										'default'   => false,
										'parameters' => Array(
															'allow_empty' => false
														),
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

		$this->vars['parameter_map'] = Array(
										'added'	=> '0.4',
										'type'  => 'parameter_map',
										'default' => Array(),
										'parameters' => Array(
											'parameters'=> Array(
												'root_node' => 'Replacement Root node for the listing (must be a child of the static root node)',
												)
											)
										);

		$this->vars['column_layout']	= Array(
										'added'   => '0.7',
										'type'    => 'serialise',
										'default' => Array('0' => '1'),
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
			bam('STARTING ASSET LISTING UPGRADE - VERSION 0.1');
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
			bam('ASSET LISTING UPGRADE COMPLETE - VERSION 0.1');
		}//end version 0.1 upgrade

		return true;

	}//end _upgrade()


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
					'event_name'                 => 'requestKeywords',
					'broadcast_type_code'        => 'content_type',
					'broadcast_strict_type_code' => false,
					'options'	=> Array(
						'side_of_link' => 'major',
						'indirect'     => true,
						'is_exclusive' => null,
						'is_dependant' => 1,
						'value'        => '',
						'link_type'    => SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


}//end class

?>
