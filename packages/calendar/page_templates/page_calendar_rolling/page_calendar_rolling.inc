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
* $Id: page_calendar_rolling.inc,v 1.10 2006/06/08 05:10:28 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_PACKAGES_PATH.'/calendar/lib/calendar_common.inc';

/**
* Page_Calendar_Rolling
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Rolling extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Calendar_Rolling($assetid=0)
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

		$heading_tag = $this->attr('day_heading_tag');
		$heading_format = $this->attr('day_heading_format');

		$all_events = $this->_getEvents();
		$all_ids = Array();
		foreach ($all_events as $day => $event_list) {
			$all_ids = array_merge($all_ids, array_keys($event_list));
		}
		if ($somebody_doesnt_need_asset) {
			$asset_infos = $am->getAssetInfo($all_ids);
		}

		$event_list = '';
		$recurring_event_descendants = $am->getTypeDescendants('calendar_event_recurring', TRUE);
		foreach ($all_events as $date => $events) {
			$event_list .= "\n\n<".$heading_tag.'>'.date($heading_format, strtotime($date)).'</'.$heading_tag.">\n";
			foreach ($events as $id => $event) {

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

				$event_list .= replace_keywords($type_formats[$format_type], $replacements);

			}//end foreach events in day

		}//end foreach events

		return Array('event_list' => $event_list);

	}//end _getPageContentsReplacements()


	/**
	* Get the events that should be displayed in this calendar page
	* Returns: (date => array (assetid => array ( details )))
	*
	* @param string	$start_date	Start date
	* @param string	$end_date	End date
	*
	* @return array
	* @access protected
	*/
	function _getEvents($start_date=NULL, $end_date=NULL)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		if (is_null($start_date)) {
			$days_from = $this->attr('days_from');
			$days_to   = $this->attr('days_to');

			// switch if the values are the wrong way around
			if ($days_from > $days_to) {
				$tmp       = $days_from;
				$days_from = $days_to;
				$days_to   = $tmp;
			}

			$now_ts = time();
			$start_date = date('Y-m-d', $now_ts + ($days_from * 60 * 60 * 24));
			$end_date   = date('Y-m-d', $now_ts + ($days_to   * 60 * 60 * 24));
		}
		$length = (iso8601_ts($end_date) - iso8601_ts($start_date)) / (60 * 60 * 24);

		// build sql to restrict the tree locations
		$root_nodes = $this->attr('root_nodes');
		if (empty($root_nodes)) {
			trigger_localised_error('CAL0028', E_USER_WARNING);
			return Array();
		}

		// get the single events
		$start_date_stamp = floor(strtotime($start_date.' 12:00:00 GMT') / 86400);
		$end_date_stamp = $start_date_stamp + $length;
		$date_sql = '(cd.start_date_ds BETWEEN '.$db->quote($start_date_stamp).' AND '.$db->quote($end_date_stamp).') OR (cd.start_date_ds < '.$db->quote($start_date_stamp).' AND cd.end_date_ds >= '.$db->quote($start_date_stamp).')';
		$sql = Calendar_Common::getSingleEventQueryBase(array_keys($root_nodes)).' AND ('.$date_sql.')';
		$single_result = $db->getAll($sql);
		assert_valid_db_result($single_result);
		$single_result = Calendar_Common::condenseResultTreeids($single_result);

		// get the recurring events
		$sql = Calendar_Common::getRecurringEventQueryBase(array_keys($root_nodes), 'period', $start_date, (int) $length);
		$recur_result = $db->getAll($sql);
		assert_valid_db_result($recur_result);
		$recur_result = Calendar_Common::condenseResultTreeids($recur_result);

		$all_events = $single_result + $recur_result;
		$all_events = Calendar_Common::expandEventList($all_events, $start_date, $end_date);
		Calendar_Common::processExceptions($all_events);

		$index = $start_date;
		$res = Array($index => Array());
		while ($index < $end_date) {
			$index = date('Y-m-d', strtotime($index.' +1 day'));
			$res[$index] = Array();
		}

		foreach ($all_events as $id => $event) {
			$event_date = sprintf('%04d-%02d-%02d', $event['start_date_year'], $event['start_date_mon'], $event['start_date_mday']);
			$res[$event_date][strtok($id, ':')] = $event;
		}

		// finally, filter by weekdays
		$day_filter = explode('|', $this->attr('day_filter'));
		foreach ($res as $day => $day_events) {
			if (!in_array(date('D', strtotime($day)), $day_filter)) {
				unset($res[$day]);
			}
		}

		return $res;

	}//end _getEvents()


}//end class
?>
