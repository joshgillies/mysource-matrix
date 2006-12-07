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
* $Id: page_upcoming_events_list.inc,v 1.3 2006/12/07 05:51:52 bcaldwell Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_PACKAGES_PATH.'/calendar/lib/calendar_common.inc';

/**
* Page_Upcoming_Events_List
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Upcoming_Events_List extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Upcoming_Events_List($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Page($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		// create a page contents bodycopy
		$am->includeAsset('bodycopy');
		$page_contents_bc =& new Bodycopy();
		$page_contents_bc->setAttrValue('name', 'Page Contents');
		if (!$page_contents_bc->create($link)) return FALSE;

		// create a type formats folder
		$am->includeAsset('folder');
		$type_formats_folder =& new Folder();
		$type_formats_folder->setAttrValue('name', 'Type Formats');
		if (!$type_formats_folder->create($link)) return FALSE;

		// create type format bodycopies
		$tf_link = Array('asset' => &$type_formats_folder, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 0);
		foreach (Array('calendar_event_single', 'calendar_event_recurring') as $event_type) {
			$tf_link['value'] = $event_type;
			$format_bc =& new Bodycopy();
			$format_bc->setAttrValue('name', $am->getTypeInfo($event_type, 'name').' Format');
			if (!$format_bc->create($tf_link)) return FALSE;
			$format_bc = NULL;
		}

		return TRUE;

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 1, 'exclusive' => TRUE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) {
			$vars['keywords'] = Array();
		}
		$tf_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, '', 'major', TRUE);
		$tf_folder_id = $tf_folder_link['minorid'];
		$broadcaster_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id);
		if (isset($broadcaster_parents[$tf_folder_id])) {
			$tf_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($tf_folder_id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major');
			$tf_bcs = Array();
			foreach ($tf_links as $link) {
				$tf_bcs[$link['minorid']] = $link['value'];
			}
			$bc_id = reset(array_intersect(array_keys($tf_bcs), array_keys($broadcaster_parents)));
			if ($bc_id) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($tf_bcs[$bc_id]);
				$dummy_asset =& new $tf_bcs[$bc_id];
				$asset_keywords = $dummy_asset->getAvailableKeywords();
				foreach ($asset_keywords as $kw => $desc) {
					$vars['keywords'][$kw] = $desc;
				}
				return TRUE;
			}
		} else if (isset($broadcaster_parents[$this->id])) {
			// it's the contents BC
			$vars['keywords']['event_list'] = 'List of Events';
			return TRUE;
		}
		return FALSE;

	}//end onRequestKeywords()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$mm = NULL;
		$page_links = $am->getLinks($this->id, SQ_LINK_TYPE_2, Array('folder', 'bodycopy'), TRUE, 'major', NULL, TRUE);
		$tf_folder_id = NULL;
		$pb_bodycopy_id = NULL;
		foreach ($page_links as $link) {
			switch ($link['minor_type_code']) {
				case 'folder':
					$this->_tmp['tf_folder_id'] = $link['minorid'];
				break;
				case 'bodycopy':
					$pb_bodycopy_id = $link['minorid'];
				break;
			}
		}
		if (is_null($this->_tmp['tf_folder_id'])) {
			trigger_localised_error('CAL0059', E_USER_WARNING, $this->id);
			return;
		}
		if (is_null($pb_bodycopy_id)) {
			trigger_localised_error('CAL0060', E_USER_WARNING, $this->id);
			return;
		}

		$pb =& $am->getAsset($pb_bodycopy_id, 'bodycopy');
		$pb->setKeywordReplacements($this->_getPageContentsReplacements());
		$pb->printBody();

	}//end printBody()


	/**
	* Get the keyword replacements to use in the page contents bodycopy
	*
	* @return array
	* @access protected
	*/
	function _getPageContentsReplacements()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$type_format_links = $am->getLinks($this->_tmp['tf_folder_id'], SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', NULL, TRUE);
		$type_formats = Array();
		$needs_asset = Array();
		$somebody_doesnt_need_asset = FALSE;
		$metadata_keywords = Array();
		$asset_info_fields =  Array(
								'assetid',
								'type_code',
								'version',
								'name',
								'short_name',
								'status',
								'languages',
								'charset',
								'force_secure',
								'created',
								'created_userid',
								'updated',
								'updated_userid',
								'published',
								'published_userid',
							  );
		foreach ($type_format_links as $link) {
			if (!empty($link['value'])) {
				$needs_asset[$link['value']] = FALSE;
				$metadata_keywords[$link['value']] = Array();
				$bc =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
				$type_formats[$link['value']] = $bc->getRawBodycopyContent();
				$required_keywords[$link['value']] = $bc->getKeywords();
				foreach ($required_keywords[$link['value']] as $keyword) {
					if (strpos($keyword, 'asset_') !== 0) {
						$needs_asset[$link['value']] = TRUE;
					} else {
						// the keyword starts with 'asset_' but it might still require the asset
						if ((strpos($keyword, 'asset_metadata_') !== 0) && (!in_array(substr($keyword, 6), $asset_info_fields))) {
							$needs_asset[$link['value']] = TRUE;
						}
					}
					if (strpos($keyword, 'asset_metadata_') === 0) {
						$metadata_keywords[$link['value']][] = $keyword;
					}
				}
				if (!$needs_asset[$link['value']]) {
					$somebody_doesnt_need_asset = TRUE;
				}
			}
		}

		$events = $this->_getEvents();
		if ($somebody_doesnt_need_asset) {
			$asset_infos = $am->getAssetInfo(array_keys($events));
		}

		$event_list = '';
		$recurring_event_descendants = $am->getTypeDescendants('calendar_event_recurring', TRUE);
		foreach ($events as $id => $event) {
			$replacements = Array();

			$format_type = (in_array($event['type_code'], $recurring_event_descendants)) ? 'calendar_event_recurring' : 'calendar_event_single';

			// get asset keywords
			if ($needs_asset[$format_type]) {
				$asset =& $am->getAsset($id);
				foreach ($required_keywords[$format_type] as $keyword) {
					if ($keyword == 'asset_contents') {
						ob_start();
							$asset->printBody();
							$replacements[$keyword] = ob_get_contents();
						ob_end_clean();
					} else {
						$replacements[$keyword] = $asset->getKeywordReplacement($keyword);
					}
				}
			} else {
				// we can do this without the asset
				foreach ($asset_infos[$id] as $field => $value) {
					$replacements['asset_'.$field] = $value;
				}
				$replacements['asset_assetid'] = $id;
			}

			// get metadata keywords
			if (!empty($metadata_keywords[$event['type_code']])) {
				if (!empty($metadata_keywords)) {
					if (is_null($mm)) {
						$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
					}
					// get all the metadata keywords for this asset
					$metadata_values = $mm->getMetadataFieldValues($id, $metadata_keywords);

					foreach ($metadata_values as $field => $value) {
						$replacements['asset_metadata_'.$field] = $value;
					}
				}
			}

			$entry = $type_formats[$format_type];
			replace_keywords($entry, $replacements);
			$event_list .= $entry;

		}//end foreach events

		return Array('event_list' => $event_list);

	}//end _getPageContentsReplacements()


	/**
	* Get the events that should be displayed in this calendar page
	* Returns: (array (assetid => array ( details )))
	*
	* @return array
	* @access protected
	*/
	function _getEvents()
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		// build sql to restrict the tree locations
		$root_nodes = $this->attr('root_nodes');
		if (empty($root_nodes)) {
			trigger_localised_error('CAL0028', E_USER_WARNING);
			return Array();
		}

		$start_ts = time();
		$horizon_ts = strtotime('+'.$this->attr('horizon').'days');
		$all_events = Array();

		while ((count($all_events) < $this->attr('num_events')) && ($start_ts < $horizon_ts)) {

			$end_ts = strtotime('+'.$this->attr('num_events').' days', $start_ts);

			// get the single events
			$date_sql = '(cd.start_date_ts BETWEEN '.$db->quote($start_ts).' AND '.$db->quote($end_ts).') OR (cd.start_date_ts < '.$db->quote($start_ts).' AND cd.end_date_ts >= '.$db->quote($start_ts).')';
			$sql = Calendar_Common::getSingleEventQueryBase(array_keys($root_nodes)).' AND ('.$date_sql.')';
			$single_result = $db->getAll($sql);
			assert_valid_db_result($single_result);
			$single_result = Calendar_Common::condenseResultTreeids($single_result);

			// get the recurring events
			$sql = Calendar_Common::getRecurringEventQueryBase(array_keys($root_nodes), 'period', date('Y-m-d', $start_ts), $this->attr('num_events'));
			$recur_result = $db->getAll($sql);
			assert_valid_db_result($recur_result);
			$recur_result = Calendar_Common::condenseResultTreeids($recur_result);

			$new_events = $single_result + $recur_result;
			$new_events = Calendar_Common::expandEventList($new_events, date('Y-m-d', $start_ts), date('Y-m-d', $end_ts));

			Calendar_Common::processExceptions($new_events);

			$all_events += $new_events;

			$start_ts = $end_ts - 1; // for next loop if necessary

		}
		uasort($all_events, Array('Calendar_Common', 'compareStartDates'));
		$res = Array();
		$i = 0;
		foreach ($all_events as $id => $event) {
			if ($i >= $this->attr('num_events')) break;
			$res[strtok($id, ':')] = $event;
			$i++;
		}
		return $res;

	}//end _getEvents()


}//end class
?>
