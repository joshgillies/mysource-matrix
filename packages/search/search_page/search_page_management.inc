<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: search_page_management.inc,v 1.52 2006/12/07 02:46:11 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Page_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.52 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Search_Page_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['group_by']['default'] = 'grouped';

		$this->vars['proxy_types'] = Array(
										'added'		=> '0.15',
										'type'		=> 'serialise',
										'default'	=> Array(),
									 );

		$this->vars['sis_button_text'] = Array(
											'added'			=> '0.5',
											'type'			=> 'text',
											'default'		=> 'Search',
											'description'	=> 'Text that would appear on the search in search submit button',
										 );

		$this->vars['fields'] = Array(
									'added'			=> '0.1',
									'type'			=> 'serialise',
									'default'		=> Array(),
									'description'	=> 'Search fields.',
								);

		$this->vars['categories'] = Array(
										'added'			=> '0.4',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Search categories.',
									);

		$this->vars['no_category_text'] = Array(
											'added'			=> '0.4',
											'type'			=> 'text',
											'default'		=> 'No Category Selected',
											'description'	=> 'The text that appears for a "no category selected" options.',
										  );

		$this->vars['search_types'] = Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Searchable asset types',
									  );

		$this->vars['search_styles'] = Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Styles of the search fields',
									   );

		$this->vars['use_picker'] = Array(
										'added'			=> '0.6',
										'type'			=> 'boolean',
										'default'		=> FALSE,
										'description'	=> 'Use the JS date picker or not',
										'parameters'	=> Array('allow_empty' => FALSE),
									);

		$this->vars['logic'] = Array(
								'added'			=> '0.1',
								'type'			=> 'selection',
								'default'		=> 'AND',
								'description'	=> 'Whether to include all or any fields.',
								'parameters'	=> Array(
													'multiple'		=> FALSE,
													'allow_empty'	=> FALSE,
													'options'		=> Array(
																		'AND'	=> 'Match on ALL fields',
																		'OR'	=> 'Match on at least ONE field',
																	   ),
												   ),
							   );

		$this->vars['allow_empty_search'] = Array(
												'added'			=> '0.12',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'If true, empty search query will search for everything',
												'parameters'	=> Array('allow_empty' => FALSE),
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
		if (!parent::_upgrade($current_version)) return FALSE;

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		if (version_compare($current_version, '0.1', '<=')) {
			// version 0.1->0.2
			// changed the root assetid from storing an assetid to storing a NOTICE link
			// to the root assetid - that way if the root asset is purged it automatically deletes
			// the association
			pre_echo('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.1');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren(1, $this->getAssetType(), FALSE);
			foreach ($children as $assetid => $type) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;

				// get the current root asset id and make sure it exists
				$current_root_assetid = $asset->attr('root_assetid');
				if (empty($current_root_assetid)) continue;
				$root_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_root_assetid));

				// create the notice link
				if (!empty($root_asset_info)) {
					$root_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_root_assetid);
					if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $root_asset, SQ_LINK_NOTICE, 'root')) {
						pre_echo('SEARCH PAGE UPGRADE FAILED');
						return FALSE;
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			pre_echo('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.1');
		}//end version 0.1 upgrade

		if (version_compare($current_version, '0.5', '<=')) {
			// version 0.5->0.6
			// changed the format of the fields array
			pre_echo('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.5');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren(1, $this->getAssetType(), FALSE);
			foreach ($children as $assetid => $type) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;

				$fields = $asset->attr('fields');

				$new_fields = Array();
				foreach ($fields as $field_name => $field_data) {
					// strip out the additional name field
					if (isset($field_data['name'])) {
						unset($field_data['name']);
					}

					// metadata fields need their params reworked
					if ($field_data['type'] == 'metadata') {
						$field_data['params'] = Array('assetid' => $field_data['params']['metadata_field']['assetid']);
					}

					if ($field_data['type'] == 'asset_attrib') {
						$new_params = Array();
						$new_params['asset_type'] = $field_data['params']['searchtype'];

						// [0] - name, [1] - type, [2] - id
						$attr_parts = explode('__', $field_data['params']['type_attr']);
						$new_params['attrid'] = $attr_parts[2];

						$field_data['params'] = $new_params;
					}

					$new_fields[$field_name] = $field_data;
				}
				$asset->setAttrValue('fields', $new_fields);
				if (!$asset->saveAttributes()) {
					trigger_localised_error('SCH0018', E_USER_WARNING);
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}//end foreach
			pre_echo('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.5');
		}//end if version 0.5 upgrade


		if (version_compare($current_version, '0.7', '<=')) {
			// version 0.7->0.8
			// changed the format of the fields array
			pre_echo('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.7');
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				// copy over the number of results per page to the corresponding
				// listing engine variable
				$asset->setAttrValue('num_per_page',  $asset->attr('results_per_page'));

				// create the type and posistion formatting folders
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

				$sub_assets= Array(
								'type_formats'		=> 'folder',
								'position_formats'	=> 'folder',
							 );

				$type_formats = NULL;
				foreach ($sub_assets as $name => $type) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, $type, TRUE, $name);
					if (!is_null($link)) {
						trigger_localised_error('SCH0019', E_USER_WARNING, ucwords(str_replace('_', ' ', $name.' '.$type)));
						continue;
					}

					$sub_asset =& new $type();
					$copy_link = Array('asset' => &$asset, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

					$sub_asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
					if (!$sub_asset->create($copy_link)) return FALSE;

					if ($name == 'type_formats') {
						$type_formats = $sub_asset;
					}

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($sub_asset);
					unset($sub_asset);
				}

				// NOW, convert the 'result list' formats stuff into the default
				// type format
				$ri_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', FALSE, 'result_element');

				if (is_null($ri_link)) {
					trigger_localised_error('SCH0020', E_USER_WARNING);
					continue;
				} else {

					$ri_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($ri_link['minorid']);

					$ri_asset->setAttrValue('name', 'Default Format');
					$ri_asset->saveAttributes();

					$updates = Array(
								'majorid'	=> $db->quote($type_formats->id),
								'value'		=> $db->quote('default_format'),
							   );

					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$db = $GLOBALS['SQ_SYSTEM']->db;
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					$sql = 'UPDATE
								sq_ast_lnk
							SET
								majorid	= '.$db->quoteSmart($type_formats->id).',
								value	= '.$db->quoteSmart('default_format').'
							WHERE
								linkid = '.$db->quoteSmart($ri_link['linkid']);

					$result = $db->query($sql);
					assert_valid_db_result($result);

					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					$db = $GLOBALS['SQ_SYSTEM']->db;

				}

				// now move search types if any are set
				$types = $asset->attr('search_types');
				if ((count($types) > 0) && !is_bool(reset($types))) {

					// search_types attribute is now listing engine's
					// types attribute
					$types = $asset->attr('search_types');

					// flip the types array to get typecodes as keys, then set them
					// to inherit types (as this was what Search Manager used to do)
					$types = array_flip($types);
					foreach ($types as $type_code => $value) {
						$types[$type_code] = TRUE;
					}

					$asset->setAttrValue('types', $types);

				}


				if (!$asset->saveAttributes()) {
					trigger_localised_error('SCH0021', E_USER_WARNING);
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}//end foreach

			pre_echo('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.7');

		}//end if version 0.7 upgrade


		if (version_compare($current_version, '0.9', '<=')) {
			// version 0.9->0.10
			// create a new No Results bodycopy for each search_page
			pre_echo('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.9');
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				// TODO: FIXME: cvs -D, test upgrade
				$copy_link = Array(
								'asset'			=> &$asset,
								'link_type'		=> SQ_LINK_TYPE_2,
								'is_dependant'	=> 1,
								'is_exclusive'	=> 1,
								'value'			=> 'no_results',
							 );

				$bodycopy =& new Bodycopy();
				$bodycopy->setAttrValue('name', 'No Results Page Layout');
				if (!$bodycopy->create($copy_link)) {
					trigger_localised_error('SCH0023', E_USER_WARNING);
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			pre_echo('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.9');
		}//end if version 0.9 upgrade

		if (version_compare($current_version, '0.13', '<')) {
			pre_echo('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.12');
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$should_release_lock = (1 == $GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'attributes'));
				$asset->setAttrValue('statuses', $asset->attr('search_status'));
				$asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				if ($should_release_lock) {
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'attributes');
				}
			}
			pre_echo('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.12');
		}//end version 0.13 upgrade

		if (version_compare($current_version, '0.14', '<')) {
			pre_echo('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.13');
			// version 0.13->0.14
			// changing the default value of attr 'group_by')
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$asset->setAttrValue('group_by', 'grouped');
				$asset->saveAttributes();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}

			pre_echo('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.13');
		}//end version 0.12 upgrade

		if (version_compare($current_version, '0.15', '<')) {
			pre_echo('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.14');
			// version 0.14 -> 0.15
			// changed the format of the attr categories
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$old_cats = $asset->attr('categories');
				foreach ($old_cats as $i => $v) {
					$old_cats[$i]['full_name'] = $v['name'];
				}
				$new_cats['search'] = Array('options' => $old_cats);
				$sql = 'SELECT custom_val
						FROM sq_ast_attr_val
						WHERE assetid = '.$db->quote($asset->id).'
							AND attrid = (SELECT attrid
										  FROM sq_ast_attr
										  WHERE name='.$db->quote('no_category_text').'
											AND type_code = '.$db->quote('search_page').'
										 )
						';
				$unselected_text = $db->getOne($sql);
				assert_valid_db_result($unselected_text);

				$new_cats['search']['unselected_text'] = empty($unselected_text) ? '' : $unselected_text;
				$asset->setAttrValue('categories', $new_cats);
				$asset->saveAttributes();

				// change the values on the root node links
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_NOTICE, '', TRUE);
				foreach ($links as $link) {
					if (!empty($link['value']) && (0 === strpos($link['value'], 'root_'))) {
						$GLOBALS['SQ_SYSTEM']->am->updateLink($link['linkid'], NULL, 'root_search_'.substr($link['value'], 5));
					}
				}

				// now replace the category_list keyword with search_category_chooser in all html content underneath
				$content_types = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, 'content_type', FALSE);
				foreach ($content_types as $id => $type) {
					$ct =& $GLOBALS['SQ_SYSTEM']->am->getAsset($id, $type);
					$html = $ct->attr('html');
					$html = str_replace('%category_list%', '%search_category_chooser%', $html);
					$ct->setAttrValue('html', $html);
					$ct->saveAttributes();
				}
			}//end foreach
		}//end if

		if (version_compare($current_version, '0.16', '<')) {
			pre_echo('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.15');
			// fields format changed in v0.16
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				pre_echo("Upgrading Search Page #$assetid");
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$new_fields = Array();
				foreach ($asset->attr('fields') as $field_name => $field_data) {
					$new_fields[$field_name] = Array();
					$new_fields[$field_name]['name'] = $field_data['name'];
					$new_fields[$field_name]['data_sources'] = Array();
					if ($field_data['type'] == 'asset_date') {
						$new_fields[$field_name]['data_sources'][] = Array(
																		'type'		=> 'standard',
																		'params'	=> Array(
																						'field'	=> $field_data['params']['date_type'],
																					   ),
																	 );
					} else {
						$new_fields[$field_name]['data_sources'][] = Array(
																		'type'		=> $field_data['type'],
																		'params'	=> array_get_index($field_data, 'params', Array()),
																	 );
					}
				}
				$asset->setAttrValue('fields', $new_fields);
				$asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			pre_echo('FINISHED SEARCH PAGE UPGRADE FROM VERSION 0.15');
		}//end if

		if (version_compare($current_version, '0.18', '<')) {
			pre_echo('STARTING SEARCH PAGE UPGRADE - FROM VERSION '.$current_version.' TO 0.18');
			// 0.18 - Replace v3.6 Relevance fields with v3.8+ (?) versions
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			pre_echo('Updating '.count($children).' Search Page(s)...');
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$sort_by = $asset->attr('sort_by');

				foreach ($sort_by as $key => $value) {
					if ($value['type'] == 'field') {
						$field_name = $value['params']['field'];

						// If this is Relevance, change old v3.6 field name to
						// v3.8+ one (which uses 'score')
						if ($field_name == '__relevance__') {
							$sort_by[$key]['params']['field'] = 'score';
						}
					}
				}

				$asset->setAttrValue('sort_by', $sort_by);
				$asset->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			pre_echo('FINISHED SEARCH PAGE UPGRADE TO VERSION 0.18');
		}//end if

		return TRUE;

	}//end _upgrade()


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


}//end class

?>