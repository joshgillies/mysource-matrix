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
* $Id: funnelback_search_page_edit_fns.inc,v 1.12 2012/08/30 00:58:43 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Funnelback_Search_Page_Edit_Fns
*
*     Edit functions for funnelback search page
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Search_Page_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['lock_type'] = 'content';
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Returns true if integrated search is enabled
	*
	* @param object	&$owner	the search manager
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function integratedSearch(&$owner, $prefix)
	{
		return ($owner->attr('search_type') == 'integrated');

	}//end integratedSearch()


	/**
	* Paint links to the default formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Edit Format') ?></th>

				<th><?php echo translate('Note') ?></th>

			</tr>
			<?php
			$this->_paintDefaultFormatLink($asset->id, 'initial', translate('This format defines the layout for the initial funnelback search page'));

			$this->_paintDefaultFormatLink($asset->id, 'results', translate('This format defines the layout for the funnelback search results'));

			$this->_paintDefaultFormatLink($asset->id, 'no_results', translate('This format defines the layout for the page returned when no results are found'));


			$folder = $asset->getFolder('type_format');
			$this->_paintDefaultFormatLink($folder->id, 'default_format', translate('This is the default format for all results in the list if they do not have a customised format'));

			$this->_paintDefaultFormatLink($folder->id, 'featured_format', translate('This format defines how the featured results are to be displayed.'));

			?>
		</table>
		<?php

		return FALSE;

	}//end paintDefaultFormatLinks()


	/**
	* Paint links to the fluster formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFlusterFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Edit Format') ?></th>

				<th><?php echo translate('Note') ?></th>

			</tr>
			<?php
			$folder = $asset->getFolder('fluster_format');
			$this->_paintDefaultFormatLink($folder->id, 'context_format', translate('This is the layout of how the contextual navigation results looks.'));

			$this->_paintDefaultFormatLink($folder->id, 'cluster_format', translate('This is the layout of how each contextual navigation element looks.'));

			$this->_paintDefaultFormatLink($folder->id, 'navigation_format', translate('This is the layout of how the contextual navigation looks.'));

			?>
		</table>
		<?php

		return FALSE;

	}//end paintFlusterFormatLinks()


	/**
	* Paint links to the spell formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSpellFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Edit Format') ?></th>

				<th><?php echo translate('Note') ?></th>

			</tr>
			<?php
			$folder = $asset->getFolder('spell_format');
			$this->_paintDefaultFormatLink($folder->id, 'spell_format', translate('This is the layout of how the spelling suggestion looks.'));

			?>
		</table>
		<?php

		return FALSE;

	}//end paintSpellFormatLinks()


	/**
	* Prints the table containing the links to the bodycopies (and default type format)
	*
	* @param string	$root_assetid			the id of the asset to look under for bodycopies; NULL indicates the listing_engine asset itself
	* @param string		$bodycopy_name			the name of the bodycopy to print, eg. 'no_results'
	* @param string		$bodycopy_description	the description to label the bodycopy with
	* @param boolean	$bodycopy_description	print the delete control or not
	* @param string		$prefix					the prefix of the asset
	* @param string		$type_code				the type_code of the format to the print (by default, 'bodycopy')
	* @param string		$screen					the edit screen to link to
	*
	* @return void
	* @access private
	*/
	function _paintDefaultFormatLink($root_assetid, $bodycopy_name, $bodycopy_description, $delete_control=FALSE, $prefix='', $type_code='bodycopy', $screen='contents')
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($root_assetid, SQ_LINK_TYPE_2, $type_code, TRUE, $bodycopy_name);
		if (!empty($link)) {
			$info = Array($link['minorid'] => 'contents');
			$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
			?>
			<tr>
				<td>
					<?php echo get_asset_tag_line($link['minorid'], $screen); ?>
				</td>
				<td>
					<?php echo $bodycopy_description; ?>
				</td>
				<?php
				if ($delete_control) {
					echo '<td>';
					check_box($prefix.'_delete['.$bodycopy_name.']');
					echo '</td>';
				}//end if
				?>
			</tr>
			<?php
		}//end if

	}//end _paintDefaultFormatLink()


	/**
	* Paint the interface for editing custom type formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypeFormats(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$folder = $asset->getFolder('type_format');
		$formats = $asset->getFormats('type_format');

		if (count($formats) > 2) {
			?>
			<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('Edit Format') ?></th>

					<th><?php echo translate('Note') ?></th>

					<?php
					if ($write_access) {
						?>
						<th><?php echo translate('Delete?') ?></th>

						<?php
					}
					?>
				</tr>
				<?php
				foreach ($formats as $minorid => $value) {
					if ($value == 'default_format') continue;
					if ($value == 'featured_format') continue;

					$this->_paintDefaultFormatLink($folder->id, $value, sprintf(translate('This is the customised layout for the %s type formats.'), strtoupper($value)), $write_access, $prefix);

				}//end foreach
				?>
			</table>
			<?php
		} else {
			echo translate('No Type Formats Defined');

		}//end if cutomised formats

		if ($write_access) {
			echo '<p><b>'.translate('New Type:').'</b> ';

			text_box($prefix.'_new_type', '');
			echo '</p>';
		}
		$o->note(translate('Enter a file type (eg. pdf, html, doc) to use the customised format for that file type. File types can be exposed by <b>result_type</b> keyword'));

		return TRUE;

	}//end paintTypeFormats()


	/**
	* Process the interface for editing custom type formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTypeFormats(&$asset, &$o, $prefix)
	{
		$folder = $asset->getFolder('type_format');

		$new_type = array_get_index($_POST, $prefix.'_new_type', '');
		$deleted_formats = array_get_index($_POST, $prefix.'_delete', Array());

		// User wants to delete and create the same one? No point wasting time, just leave it and move on
		if (isset($deleted_formats[$new_type])) {
			unset($deleted_formats[$new_type]);
			$new_type = '';
		}//end if

		foreach ($deleted_formats as $type => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $type);
			if (!empty($current_link)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
					trigger_localised_error('FNB0006', E_USER_WARNING, $type);
				}//end if
			}//end if
		}//end foreach

		if (!empty($new_type)) {
			// check that this type isnt already customised
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $new_type);
			if (empty($current_link)) {
				// create a new format bodycopy for this type
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

				$copy_link = Array('asset' => &$folder, 'value' => $new_type, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

				$bodycopy = new Bodycopy();
				$bodycopy->setAttrValue('name', strtoupper($new_type).' Format');
				$copy_args = Array('content' => $asset->_getDefaultBodycopyContent('custom_format'));

				if (!$bodycopy->create($copy_link, $copy_args)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}//end if
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			} else {
				trigger_localised_error('FNB0005', E_USER_WARNING, $new_type);
			}//end if
		}//end if

		return FALSE;

	}//end processTypeFormats()


	/**
	* Paints the search options for choosing the url/collections
	*
	* @param object		&$asset		asset being painted
	* @param object		&$o			backend outputter
	* @param string		$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintFunnelbackOptions(&$asset, &$o, $prefix)
	{
		$wa                   = $asset->writeAccess('attributes');
		$search_type          = $asset->attr('search_type');
		$web_url              = $asset->attr('funnelback_url');
		$collection           = $asset->attr('collection');
		$fm                   = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
		$all_collections      = $fm->getCollections();
		$local_collections    = Array(''=>translate('No collection selected'));

		$squiz_search_options = $this->_getSquizSearchSystems();
		$suiteid              = $asset->attr('systemid');
		$systemname           = array_get_index($squiz_search_options, $suiteid, '');
		$squiz_collections    = $this->_getSquizSearchCollections($suiteid);
		foreach ($all_collections as $all_col_id => $all_col_info) {
			$local_collections[$all_col_id] = array_get_index($all_col_info, 'name', '');
		}//end foreach
		if ($search_type == 'integrated') {
			$hidden_web = 'style="display:none;"';
			$hidden_local = '';
			$hidden_squiz = 'style="display:none;"';
		} else if ($search_type == 'squiz') {
			$hidden_web = 'style="display:none;"';
			$hidden_local = 'style="display:none;"';
			$hidden_squiz = '';
		} else {
			$hidden_web = '';
			$hidden_local = 'style="display:none;"';
			$hidden_squiz = 'style="display:none;"';
		}//end if
		$object_type  = 'this.form.'.$prefix.'_search_type.value';
		$object_url1  = 'this.form.'.$prefix.'_search_url_web';
		$object_url2  = 'document.getElementById(\''.$prefix.'_search_url_local\')';
		$object_url3  = 'this.form.'.$prefix.'_search_url_squiz';
		$object_url4  = 'document.getElementById(\''.$prefix.'_search_mode_hosted\')';
		$object_coll1 = 'this.form.'.$prefix.'_search_collection_web';
		$object_coll2 = 'this.form.'.$prefix.'_search_collection_local';
		$object_coll3 = 'this.form.'.$prefix.'_search_collection_squiz';
		$object_coll4 = 'this.form.'.$prefix.'_search_mode';
		$js = 'onChange="chooseFunnelbackSearchType('.$object_type.', '.$object_url1.', '.$object_url2.', '.$object_url3.', '.$object_url4.', '.$object_coll1.', '.$object_coll2.', '.$object_coll3.', '.$object_coll4.');"';

		$o->openField('Search Type');
			?>
			<script type="text/javascript">
				function chooseFunnelbackSearchType(type, url1, url2, url3, url4, coll1, coll2, coll3, coll4) {
					if (type == 'web' || type == 'hosted') {
						url1.style.display='block';
						url1.disabled=false;
						url2.style.display='none';
						url2.disabled=true;
						url3.style.display='none';
						url3.disabled=true;
						coll1.style.display='block';
						coll1.disabled=false;
						coll2.style.display='none';
						coll2.disabled=true;
						coll3.style.display='none';
						coll3.disabled=true;
                        if (type == 'hosted') {
                            url4.style.display='none';
	    					url4.disabled=true;
	    					coll4.style.display='block';
		    				coll4.disabled=false;
                        } else {
                            url4.style.display='block';
	    					url4.disabled=false;
	    					coll4.style.display='none';
		    				coll4.disabled=true;
                        }
					} else if (type == 'squiz') {
						url1.style.display='none';
						url1.disabled=true;
						url2.style.display='none';
						url2.disabled=true;
						url3.style.display='block';
						url3.disabled=false;
						url4.style.display='block';
	    				url4.disabled=true;
						coll1.style.display='none';
						coll1.disabled=true;
						coll2.style.display='none';
						coll2.disabled=true;
						coll3.style.display='block';
						coll3.disabled=false;
    					coll4.style.display='none';
	    				coll4.disabled=true;
					} else {
						url1.style.display='none';
						url1.disabled=true;
						url2.style.display='block';
						url2.disabled=false;
						url3.style.display='none';
						url3.disabled=true;
						url4.style.display='block';
	    				url4.disabled=true;
						coll1.style.display='none';
						coll1.disabled=true;
						coll2.style.display='block';
						coll2.disabled=false;
						coll3.style.display='none';
						coll3.disabled=true;
    					coll4.style.display='none';
	    				coll4.disabled=true;
					}
				}
			</script>
			<?php
			$type_attribute = $asset->getAttribute('search_type');
			$type_options = $type_attribute->getOptions();
			if ($wa) {
				echo combo_box($prefix.'_search_type', $type_options, FALSE, $search_type, 0, $js);
			} else {
				echo $type_options[$search_type];
			}
		$o->note(translate('Integrated Search uses the Funnelback OEM version installed into Matrix. Funnelback Server Search can be configured to access a Funnelback Server by adding in the URL of the server. Collections will be displayed based on the type of search eg. an funnelback server search will allow you to type the collections in, while the integrated search will display the collections defined in Funnelback Manager.'));

		$o->closeField();
		$o->openField('Mode to use');
            $mode = $asset->attr('hosted_mode');
            $modes = Array('0' => 'Classic', '1' => 'Advanced');
            if($wa) {
				echo combo_box($prefix.'_search_mode', $modes, FALSE, $mode, 0, $hidden_web);
				echo '<span id="'.$prefix.'_search_mode_hosted" '.$hidden_local.'>';
                echo 'Not Available';
                echo '</span>';
            } else {
                echo $modes[$mode];
            }
		$o->closeField();
		$o->openField('Search URL');
			if ($wa) {
				echo text_box($prefix.'_search_url_web', $web_url, '', '', FALSE, $hidden_web);
				echo '<span id="'.$prefix.'_search_url_local" '.$hidden_local.'>';
				echo 'Integrated Search';
				echo '</span>';
				echo combo_box($prefix.'_search_url_squiz', $squiz_search_options, FALSE, $suiteid, 0, $hidden_squiz);
			} else {
				if ($search_type == 'integrated') {
					echo 'Integrated Search';
				} else if ($search_type == 'squiz') {
					echo $systemname;
				} else {
					echo $web_url;
				}//end if
			}//end if
		$o->closeField();
		$o->openField('Collection');
			if ($wa) {
				echo text_box($prefix.'_search_collection_web', $collection, '', '', FALSE, $hidden_web);
				echo combo_box($prefix.'_search_collection_local', $local_collections, FALSE, $collection, 0, $hidden_local);
				echo combo_box($prefix.'_search_collection_squiz', $squiz_collections, FALSE, $collection, 0, $hidden_squiz);
			} else {
				if ($search_type == 'integrated') {
					if (empty($collection) || !isset($local_collections[$collection])) {
						echo translate('No collection selected');

					} else {
						echo $local_collections[$collection];
					}//end if
				} else if ($search_type == 'squiz') {
					if (empty($collection) || !isset($squiz_collections[$collection])) {
						echo translate('No collection selected');

					} else {
						echo $squiz_collections[$collection];
					}//end if
				} else {
					echo $collection;
				}//end if
			}//end if
		$o->closeField();

		return TRUE;

	}//end paintFunnelbackOptions()


	/**
	* Processes the search options for choosing the url/collections
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processFunnelbackOptions(&$asset, &$o, $prefix)
	{
		// Save the values based on the type
		$wa = $asset->writeAccess('attributes');
		$status = FALSE;
		$type = array_get_index($_POST, $prefix.'_search_type', 'integrated');
		if ($wa) {
			if ($type == 'integrated') {
				$collection = array_get_index($_POST, $prefix.'_search_collection_local', '');
				$fm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('funnelback_manager');
				$all_collections = $fm->getCollections();
				// In case, the collection does not match the loaded collections
				if (!array_key_exists($collection, $all_collections)) {
					$collection = '';
				}//end if
				$url = '';
				$suiteid = '';
			} else if ($type == 'squiz') {
				$collection  = array_get_index($_POST, $prefix.'_search_collection_squiz', '');
				$suiteid	 = array_get_index($_POST, $prefix.'_search_url_squiz', '');
				$url		 = '';
				$systems	 = $this->_getSquizSearchSystems();
				$collections = $this->_getSquizSearchCollections($suiteid);
				if (!array_key_exists($suiteid, $systems)) {
					$suiteid = '';
				}

				if (!array_key_exists($collection, $collections)) {
					$collection = '';
				}
			} else if ($type == 'hosted') {
				$collection = array_get_index($_POST, $prefix.'_search_collection_web', '');
				$url = array_get_index($_POST, $prefix.'_search_url_web', '');
				$suiteid = '';
			} else {
				$collection = array_get_index($_POST, $prefix.'_search_collection_web', '');
				$url = array_get_index($_POST, $prefix.'_search_url_web', '');
				$suiteid = '';
			}//end if

			// get the mode value irrespective of the search type
			$mode = array_get_index($_POST, $prefix.'_search_mode', 0);
			$asset->setAttrValue('search_type', $type);
			$asset->setAttrValue('funnelback_url', $url);
			$asset->setAttrValue('systemid', $suiteid);
			$asset->setAttrValue('collection', $collection);
			$asset->setAttrValue('hosted_mode', $mode);
			$status = $asset->saveAttributes();
		}//end if

		return $status;

	}//end processFunnelbackOptions()


	/**
	 * Get a list of the Squiz Search known by this system.
	 *
	 * @return array
	 * @access private
	 */
	private function _getSquizSearchSystems()
	{
		$systems = array();
		$suitem  = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager', TRUE);
		if ($suitem !== NULL) {
			$products = $suitem->getProducts(FALSE, FALSE);
			foreach ($products as $product) {
				if ($product['type'] == 'Squiz Search' && $product['status'] == 'live') {
					$productname = array_get_index($product['connection'], 'name', '');
					$systems[$product['suiteid']] = $productname;
				}
			}//end foreach
		}//end if

		return $systems;

	}//end _getSquizSearchSystems()


	/**
	 * Get a list of the collections on a Squiz Search system.
	 *
	 * @param string $suiteid The ID of the suite product.
	 *
	 * @return array
	 * @access private
	 */
	private function _getSquizSearchCollections($suiteid='')
	{
		$collections = array();
		$suitem      = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('suite_manager', TRUE);
		if ($suitem !== NULL) {
			$system      = $suitem->getProduct($suiteid);
			if (empty($system) === FALSE) {
				$productUrl = array_get_index($system, 'url', '');
				$request    = array();
				$response   = $suitem->sendRequest($suiteid, 'json', 'SquizSearch', 'getAvailableCollections', $request);
				if (isset($response['curlInfo']['http_code']) === TRUE && $response['curlInfo']['http_code'] == 200) {
					$resultData = $response['result'];
					$resultData = (array) $suitem->decodeJson($resultData);
					$results    = (array) $resultData['result'];
					if (empty($results) === FALSE) {
						foreach ($results as $k => $v) {
							$collections[$k] = $v;
						}
					}
				}//end if
			}//end if
		}//end if

		return $collections;

	}//end _getSquizSearchCollections()


	/**
	* Paints the sort options
	*
	* @param object		&$asset		asset being painted
	* @param object		&$o			backend outputter
	* @param string		$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintSort(&$asset, &$o, $prefix)
	{
		$sort = $asset->attr('sort_order');
		$sort_options = $asset->getSortOptions();

		if ($asset->writeAccess('attributes')) {
			combo_box($prefix.'_sort_order', $sort_options, FALSE, $sort);
		} else {
			if (!isset($sort_options[$sort])) {
				$sort = 'rank';
			}//end if
			echo $sort_options[$sort];
		}//end if

		return TRUE;

	}//end paintSort()


	/**
	* Processes the sort options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processSort(&$asset, &$o, $prefix)
	{
		// Save the values
		if ($asset->writeAccess('attributes')) {
			$processed_value = array_get_index($_POST, $prefix.'_sort_order', 'rank');
			$current_value = $asset->attr('sort_order');
			if ($processed_value != $current_value) {
				$asset->setAttrValue('sort_order', $processed_value);
				$asset->saveAttributes();
			}//end if
		}//end if

		return TRUE;

	}//end processScope()


	/**
	* Paints the default scope options
	*
	* @param object		&$asset		asset being painted
	* @param object		&$o			backend outputter
	* @param string		$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultScope(&$asset, &$o, $prefix)
	{
		$scope   = $asset->attr('scope');
		$default = $asset->attr('default_scope');

		if ($asset->writeAccess('attributes')) {
			echo combo_box($prefix.'_default_scope', $scope, FALSE, $default);
		} else {
			echo (isset($scope[$default])) ? $scope[$default] : translate('No Default Scope is selected');

		}//end if

		return TRUE;

	}//end paintDefaultScope()


	/**
	* Processes the default scope options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processDefaultScope(&$asset, &$o, $prefix)
	{
		// Save the values
		$processed_value = array_get_index($_POST, $prefix.'_default_scope', '');
		if (!empty($processed_value) && $asset->writeAccess('attributes')) {
			$asset->setAttrValue('default_scope', $processed_value);
			$asset->saveAttributes();
		}//end if

		return TRUE;

	}//end processDefaultScope()


	/**
	* Paints the scope options
	*
	* @param object		&$asset		asset being painted
	* @param object		&$o			backend outputter
	* @param string		$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintScope(&$asset, &$o, $prefix)
	{
		$scope = $asset->attr('scope');

		$this->_paintOptionValue($scope, $asset, $o, $prefix);

		return TRUE;

	}//end paintScope()


	/**
	* Processes the scope options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processScope(&$asset, &$o, $prefix)
	{
		// Save the values
		if (!isset($_REQUEST[$prefix.'_options'])) return;
		$value = $_REQUEST[$prefix.'_options'];
		foreach ($value as $i => $v) {
			$value[$i] = htmlentities($v);
			if ($v == '') unset($value[$i]);
		}

		// Save the key fields
		if (!isset($_REQUEST[$prefix.'_options_keys'])) return;
		$key_value = $_REQUEST[$prefix.'_options_keys'];
		foreach ($key_value as $j => $k) {
			$key_value[$j] = htmlentities($k);
			if (empty($key_value[$j])) {
				unset($key_value[$j]);
			}//end if
		}//end foreach

		$values = Array();
		foreach ($key_value as $index => $key) {
			$values[$key] = (isset($value[$index])) ? $value[$index] : '';
		}//end foreach

		$processed_value = $values;
		if ($asset->writeAccess('attributes')) {
			$asset->setAttrValue('scope', $processed_value);
			$asset->saveAttributes();
		}//end if

		return TRUE;

	}//end processScope()


	/**
	* Print the interface for filling in a value
	*
	* @param array	$values	the values we are painting
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access private
	*/
	function _paintOptionValue($values, &$asset, &$o, $prefix)
	{
		// Check for write access
		if ($asset->writeAccess('attributes')) {
			$read_only = FALSE;
		} else {
			$read_only = TRUE;
		}

		if (!$read_only) {
			?>
			<div id="option-list-<?php echo $prefix; ?>">
				<?php
				$order = 0;
				if (count($values) > 0) {
					foreach ($values as $k => $v) {
						$k = html_entity_decode($k);
						$v = html_entity_decode($v);
						$printup = ($order < 1) ? FALSE : TRUE;
						$this->_printOptionField($k, $v, $order, $prefix, 20);
						$order++;
					}
				}
				for ($i = 0; $i < 2; $i++) {
					$k = '';
					$v = '';
					$printdown = ($i < 1) ? TRUE : FALSE;
					$this->_printOptionField($k, $v, $order, $prefix, 20);
					$order++;
				}
				?>
			</div>
			<script type="text/javascript">
			<?php
			// Yeesh... this is hackish but this essentially (attempts to)
			// correct the lack of delete-button class in front-end
			// asset builder creation
			if (!SQ_IN_BACKEND && !SQ_IN_LIMBO) {
			?>
				buttons = document.getElementsByTagName('BUTTON');
				for (i = 0; i < buttons.length; i++) {
					if (buttons[i].className == 'delete-button') {
						with (buttons[i].style) {
							backgroundImage = 'url(<?php echo sq_web_path('lib') ?>/web/images/icons/delete.png)';
							backgroundPosition = '50% 50%';
							backgroundRepeat = 'no-repeat';
							width = '16px';
							height = '16px';
							backgroundColor = 'transparent';
							border = '0px';
							cursor = 'pointer';
						}
					}
				}
			<?php
			}//end if
			?>
			// Some variables
			var optionItemPrefix = '<?php echo $prefix; ?>';

			// Functions for option list attribute
			var doExpandList = new Function('doExpandOptionList(this)');
			var doDeleteRow = new Function('doDeleteOptionListRow(this); return false;');

			function doExpandOptionList(input)
			{
				// abort if we are not the last input in the lit
				var nextInput = input.nextSibling;
				while (nextInput !== null) {
					if (nextInput.tagName == 'INPUT') {
						return;
					}
					nextInput = nextInput.nextSibling;
				}

				// abort if we and the second-last input are both empty
				var lastInput = input.previousSibling;
				var last2Input = lastInput.previousSibling;
				if (input.value == '') {
					while (lastInput !== null) {
						if (lastInput.tagName == 'INPUT') {
							last2Input = lastInput.previousSibling;
							while (last2Input !== null) {
								if (last2Input.tagName == 'INPUT') {
									if (last2Input.value == '') {
										return;
									}
									break;
								}
								last2Input = last2Input.previousSibling;
							}
							break;
						}
						lastInput = lastInput.previousSibling;
					}
				}

				var optionList = document.getElementById('option-list-<?php echo $prefix; ?>');
				var inputs = optionList.getElementsByTagName('INPUT');

				var brElements = lastInput.parentNode.getElementsByTagName('BR');
				input.parentNode.removeChild(brElements[brElements.length-1]);
				input.parentNode.appendChild(document.createTextNode(' '));
				input.parentNode.appendChild(document.createElement('BR'));

				// add the extra field
				var newInput = input.cloneNode(true);
				var newKeyInput = lastInput.cloneNode(true);
				var newId = (inputs.length/2);
				var newKeyId = (inputs.length/2);
				newKeyInput.onfocus = doExpandList;
				newKeyInput.value = '';
				newKeyInput.id = optionItemPrefix+'_options_keys['+newId+']';
				input.parentNode.appendChild(newKeyInput);
				input.parentNode.appendChild(document.createTextNode(' '));
				newInput.onfocus = doExpandList;
				newInput.value = '';
				newInput.id = optionItemPrefix+'_options['+newId+']';
				input.parentNode.appendChild(newInput);
				input.parentNode.appendChild(document.createTextNode(' '));
				var delButton = input.nextSibling;
				while (delButton.tagName != 'BUTTON') {
					delButton = delButton.nextSibling;
				}
				delButton = delButton.cloneNode(true);
				delButton.onclick = doDeleteRow;
				input.parentNode.appendChild(delButton);
				input.parentNode.appendChild(document.createTextNode(' '));

				input.parentNode.appendChild(document.createTextNode(' '));
				input.parentNode.appendChild(document.createElement('BR'));

			}

			function doDeleteOptionListRow(button)
			{
				var input = button.previousSibling;
				while (input.tagName != 'INPUT') {
					input = input.previousSibling;
				}

				var input2 = input.previousSibling;
				while (input2.tagName != 'INPUT') {
					input2 = input2.previousSibling;
				}

				if (input.value == '') return;

				// If the input is disabled, don't respond
				if (input.disabled) return;

				// Don't let the option list get down to a single element. Clear the field
				// instead, but leave it as two elements
				var inputs = input.parentNode.getElementsByTagName('INPUT');
				if (inputs.length <= 2) {
					input.value = '';
					return;
				}


				var brTag = button.nextSibling;
				while (brTag.tagName != 'BR') {
					brTag = brTag.nextSibling;
				}
				button.parentNode.removeChild(input2);
				button.parentNode.removeChild(input);
				button.parentNode.removeChild(brTag);
				button.parentNode.removeChild(button);
			}

			// attach the event handlers
			var optionList = document.getElementById('option-list-<?php echo $prefix; ?>');
			var	inputs = optionList.getElementsByTagName('INPUT');
			for (var j=0; j < inputs.length; j++) {
				inputs[j].onfocus = doExpandList;
			}
			var buttons = optionList.getElementsByTagName('BUTTON');
			for (var j=0; j < buttons.length; j++) {
				buttons[j].onclick = doDeleteRow;
			}

			lastOrder = <?php echo $order; ?>;

			</script>
			<?php
		} else {
			if (count($values) > 0) {
				echo '<ul>';
				foreach ($values as $value) {
					if (!empty($value)) {
						echo '<li>'.html_entity_decode($value).'</li>';
					}//end if
				}//end foreach
				echo '</ul>';
			}//end if
		}//end if

	}//end _paintOptionValue()


	/**
	* paint the option field
	*
	* @param string		$key		the key of the current field
	* @param string		$value		the value of the current field
	* @param string		$order		the order of the current field
	* @param string		$prefix		the prefix of the current form/field
	* @param int		$size		the size of the field
	*
	* @return string
	* @access private
	*/
	function _printOptionField($key, $value, $order, $prefix, $size='10')
	{
		// The order is key1=0, value1=1, key2=2, value2=3 etc. This is just the way they are ordered
		$current_button = ($order * 2) + 1;

		// Cut the key field down to half the value field
		$key_size = $size / 2;

		// Show the field
		?>
		<input type="text"  name="<?php echo $prefix; ?>_options_keys[]" value="<?php echo $key; ?>" id="<?php echo $prefix; ?>_options_keys[<?php echo $order; ?>]" size="<?=$key_size; ?>" />
		<input type="text"  name="<?php echo $prefix; ?>_options[]" value="<?php echo $value; ?>" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]" size="<?=$size; ?>" />
		<button type="button" tabindex="99999" class="delete-button">&nbsp;</button>
		<br />
		<?php

	}//end _printOptionField()


}//end class

?>
