<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: page_asset_listing_management.inc,v 1.40 2012/08/30 00:58:08 ewang Exp $
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
* @version $Revision: 1.40 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['all_root_nodes'] = Array(
											'added'			=> '0.14',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array('allow_empty'	=> FALSE),
										);

		$this->vars['link_value_filter'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   );
		$this->vars['link_value_filter_enabled'] = Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   );

		$this->vars['link_value_filter_logic'] = Array(
													'added'			=> '0.16',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   );

		$this->vars['parameter_map'] = Array(
										'added'			=> '0.15',
										'type'			=> 'parameter_map',
										'default'		=> Array(),
										'parameters'	=> Array(
															'parameters'	=> Array(
																				'root_node'			=> 'Replacement Root node for the listing (must be a child of the static root node)',
																				'asset_selection'	=> 'Array of specific asset IDs or selection group (specified assets must be children of the static related node)',
																			   ),
														   ),
										);
		$this->vars['dynamic_asset_selection_option'] = Array(
										'added'			=> '0.16',
										'type'			=> 'selection',
										'default'		=> 'empty_result',
										'parameters'	=> Array(
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
															'options'		=> Array(
																				'static_root'		=> 'Use static root node',
																				'empty_result'		=> 'Return empty result',
																			   ),
														   ),														   
									   );

	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


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
		parent::_upgrade($current_version);

		if (version_compare($current_version, '0.17', '<')) {
			echo('UPGRADING ASSET LISTING PAGE FROM '.$current_version."\n");
			$count = 0;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			pre_echo('Updating Asset Listing Pages...');

			// Get all Asset Listing Page assets
			$asset_listing_pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_asset_listing', FALSE, TRUE);

			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);

			foreach ($asset_listing_pages as $id => $type) {
				$asset_listing_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$get_attr_details = $asset_listing_page->getAttribute('dynamic_asset_selection_option');
				$attr_id = $get_attr_details->id;
				$sql = 'SELECT custom_val FROM sq_ast_attr_val';
				$where = ' WHERE assetid = '.MatrixDAL::quote($id);
				$where .= ' AND attrid = '.MatrixDAL::quote($attr_id);
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				$result = DAL::executePdoAll($query);
				echo 'Updating #'.$id;
				if (empty($result)) {
					if (!$asset_listing_page->setAttrValue('dynamic_asset_selection_option', 'static_root') || !$asset_listing_page->saveAttributes()) {

						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

						pre_echo('UPGRADE FAILED - Could not save attributes on asset #'.$id);
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						return FALSE;
					}//end if
					echo " done\n";
					$count++;
				} else {
					echo " not required\n";
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset_listing_page, TRUE);
				unset($asset_listing_pages);

			}//end foreach()

			pre_echo('Updated - '.$count.' asset listing page(s)...');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo('ASSET LISTING PAGES SUCCESSFULLY UPGRADED TO VERSION 0.17');
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		return TRUE;
	
	}//end _upgrade()



}//end class

?>
