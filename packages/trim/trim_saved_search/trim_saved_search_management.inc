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
* $Id: trim_saved_search_management.inc,v 1.12.6.1 2013/07/09 08:27:57 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_PACKAGES_PATH.'/trim/lib/trim_common.inc';

/**
* Trim_Saved_Search_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.12.6.1 $
* @package MySource_Matrix_Packages
* @subpackage __trim__
*/
class TRIM_Saved_Search_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function TRIM_Saved_Search_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'wsdl'					=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
												   ),
						'cached_result'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
											   		'parameters'	=> Array (
												   						'allow_empty'		=> FALSE,
												   					   ),
												   ),
						'trim_connector_id'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'assetid',
													'default'		=> '',
													'description'	=> 'The asset we will be using as content in the email',
												   ),
						'download_files'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'Whether to download the files attached with documents or not',
													'parameters'    => Array(
																		'allow_empty'   => FALSE,
												   					   ),
												   ),
						'use_downloader'		=> Array(
													'added'         => '0.1',
													'type'          => 'selection',
													'default'       => 'No',
													'description'	=> 'Whether to create the helper downloader asset',
													'is_admin'      => FALSE,
													'parameters'    => Array(
																		'allow_empty'   => FALSE,
																		'multiple'      => FALSE,
																		'options'		=> Array (
																							'Yes'	=> 'Yes',
																							'No'	=> 'No',
																						   ),
																	   ),
												   ),
                        'username'				=> Array(
		                                            'added'     => '0.1',
		                                            'type'      => 'text',
		                                            'default'   => '',
		                                           ),
                        'password'				=> Array(
		                                        	'added'     => '0.1',
		                                            'type'      => 'password',
		                                            'default'   => '',
		                                           ),
						'download_limit'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 4194304,
													'parameters'	=> Array (
																		'allow_empty'		=> FALSE,
																		'allow_negative'	=> FALSE,
																	   ),
												   ),
						'record_search_type'	=> Array (
											   		'added'			=> '0.1',
											   		'type'			=> 'serialise',
											   		'default'		=> Array(
												   						'StringSearch'	=> 'StringSearch',
												   					   ),
											   		'parameters'	=> Array (
												   						'allow_empty'		=> FALSE,
												   					   ),
											  	   ),

						'dynamic_vars'			=> Array(
													'added'		=> '0.2',
													'type'		=> 'serialise',
													'default'	=> Array (
																	'search_parent_container'	=> '',
											  	   					'search_clauses'			=> Array(
											  	   													'search_groups' => Array(),
											  	   													'global_logic'	=> 'AND',
											  	   													),
																	'result_fields'				=> Array (
																									'recTitle'	=> 'Title',
																									'recNumber'	=> 'Record Number',
																									'recRecordType'	=> 'Record Type',
																									'recDocumentType' => 'Document Type',
																									'recCreatorLoc'	=> 'Creator',
																								   ),
																	'fileTypes'					=> '',
																	'limit'						=> 30,
																	'search_sort_1'				=> 'None',
																	'search_sort_2'				=> 'None',
																	'search_sort_3'				=> 'None',
																	'sort_1_descending'			=> FALSE,
																	'sort_2_descending'			=> FALSE,
																	'sort_3_descending'			=> FALSE,
																	'record_type_filter'		=> Array(),
																   ),
												   ),
						'parameter_map'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'parameter_map',
													'default'		=> Array(),
													'parameters'	=> Array (
																		'parameters'	=> Array(),
																	   ),
												   ),
						'retrieved_fields'		=> Array (
											   		'added'			=> '0.1',
											   		'type'			=> 'serialise',
											   		'default'		=> Array(),
											   		'parameters'	=> Array (
												   						'allow_empty'		=> TRUE,
												   					   ),
											  	   ),
						'classification'		=> Array (
											  	   	'added'     => '0.2',
		                                            'type'      => 'text',
		                                            'default'   => '',
											  	   ),
						'classification_sub'	=> Array (
											  	   	'added'     => '0.2',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'Whether to include records in sub classifications',
													'parameters'    => Array(
																		'allow_empty'   => FALSE,
												   					   ),
											  	   ),
						'convert_timezone'		=> Array(
													'added'			=> '0.2',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'Whether to convert GMT datetime to local timezone',
													'parameters'    => Array(
																		'allow_empty'   => FALSE,
												   					   ),
												   	),
		    				'download_time_limit'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 3600,
													'parameters'	=> Array (
																		'allow_empty'		=> FALSE,
																		'allow_negative'	=> FALSE,
																	   ),
												   ),
		    		    		'sync_time_limit'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 7200,
													'parameters'	=> Array (
																		'allow_empty'		=> FALSE,
																		'allow_negative'	=> FALSE,
																	   ),
												   ),
					  );

	}//end constructor


    /**
    * Gets a list of all files that the bodycopy wants web accessable
    *
    * @return array
    * @access private
    */
    function _getFileList()
    {
        return Array(
                'js/ajax.js',
                'images/loader.gif',
               );

    }//end _getFileList()


    /**
	* Upgrade this asset to the current version
	*
	* @param string	$current_version	the version of the asset that is currently installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.2', '<')) {
			pre_echo('STARTING UPGRADE OF '.$asset_name.' FROM VERSION '.$current_version.' TO VERSION 0.2');
			$saved_search_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('trim_saved_search', FALSE, TRUE);
			
			$count = 0;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			pre_echo('Updating TRIM Saved Search...');
			
			$available_fields = TRIM_Common::getRecordProperties();
			$available_search_type = TRIM_Common::getRecordStringSearchClauseType();
			
			// find every trim saved search asset and update the var list
			foreach ($saved_search_ids as $id => $blank) {
				$saved_search = $GLOBALS['SQ_SYSTEM']->am->getAsset($id, $blank['type_code']);
				$var_list = $saved_search->attr('dynamic_vars');
				if(isset($var_list['fields'])) {
					$result_fields = Array();
					foreach ($var_list['fields'] as $field_name) {
						if(isset($available_fields[$field_name]))
							$result_fields[$field_name] = $available_fields[$field_name];
					}
					$var_list['result_fields'] = $result_fields;
					unset ($var_list['fields']);
				}
				if(isset($var_list['string_search_type'][0]) && isset($var_list['search_keyword'])) {
					
					$search_name = $var_list['string_search_type'][0];
					$search_caption = $available_search_type[$search_name];
					$search_keyword = $var_list['search_keyword'];
					$var_list['search_clauses'] = Array(
													'search_groups' => Array (
																		Array(
																			'content'		=>	Array (
																								Array (
																									'name'		=>	$search_name,
																									'caption'	=>	$search_caption,
																									'keyword'	=>	$search_keyword,
																								)
																							),
																			'group_logic'	=>	'AND',
																		),
																	),
													'global_logic'	=> 'AND',
												);
					unset ($var_list['string_search_type']);
					unset ($var_list['search_keyword']);
				}
				
				// Save the attribute
				$saved_search->setAttrValue('dynamic_vars', $var_list);

				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

				if (!$saved_search->saveAttributes()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

					pre_echo($asset_name.' UPGRADE FAILED - Could not save attributes');
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($saved_search);
				unset($saved_search);
				
				$count++;
			}
			
			pre_echo('Updated TRIM Saved Search - '.$count.' assets(s)...');

			
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo($asset_name.' SUCCESSFULLY UPGRADED TO VERSION 0.2');
			
			$current_version = '0.2';
		}//end if

		
		
		if (version_compare($current_version, '0.3', '<')) {
			pre_echo('STARTING UPGRADE OF '.$asset_name.' FROM VERSION '.$current_version.' TO VERSION 0.3');
			$saved_search_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('trim_saved_search', FALSE, TRUE);
			
			$count = 0;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			pre_echo('Updating TRIM Saved Search...');
			
		
			// find every trim saved search asset and update the var list
			foreach ($saved_search_ids as $id => $blank) {
				$saved_search = $GLOBALS['SQ_SYSTEM']->am->getAsset($id, $blank['type_code']);
				$var_list = $saved_search->attr('dynamic_vars');
				if(!isset($var_list['record_type_filter'])) {
					$var_list['record_type_filter'] = Array();
				}
				
				// Save the attribute
				$saved_search->setAttrValue('dynamic_vars', $var_list);

				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

				if (!$saved_search->saveAttributes()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

					pre_echo($asset_name.' UPGRADE FAILED - Could not save attributes');
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($saved_search);
				unset($saved_search);
				
				$count++;
			}
			
			pre_echo('Updated TRIM Saved Search - '.$count.' assets(s)...');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo($asset_name.' SUCCESSFULLY UPGRADED TO VERSION 0.3');
		}//end if
		
		return TRUE;

	}//end _upgrade()
	
}//end class

?>
