<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_add_assetid_remap.inc,v 1.1 2007/02/26 04:03:07 skim Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Add_URL
*
* A trigger action that sets a the value of an asset attribute
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Add_Assetid_Remap extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	* The settings used by this action are in the form:<BR />
	* This trigger simply adds an assetid at the end of site URL
	* where it belongs to.
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		if (is_null($state['asset'])) return FALSE;

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$result = Array(
					'assetid'		=> $state['asset']->id,
					'remap_urls'	=> '',
				  );

		// No URL is assigned, nothing we can do here.
		$urls = $state['asset']->getURLs();
		if (empty($urls)) return $result;

		// No site, no URL, I know, it's the same as the above.
		// Anyway I need the sites later
		$sites = $am->getParents($state['asset']->id, 'site');
		if (empty($sites)) return $result;

		// Get the list of tiny URLs we need to add
		$tiny_urls = Array();
		foreach (array_keys($sites) as $site_id) {

			$site =& $am->getAsset($site_id);
			$site_urls = $site->getSiteURLs();

			if (empty($site_urls)) {
				// The site does not have URL yet
				continue;
			} else {
				$tiny_urls = array_merge($tiny_urls, $site_urls);
			}

		}

		// Map tiny URL
		$mapped_tiny_urls = Array();
		if (!empty($tiny_urls)) {
			foreach ($tiny_urls as $tiny_url) {

				foreach ($urls as $url_info) {
					if ( ($pos = strpos($url_info['url'], $tiny_url['url'].'/')) === 0 ) {
						if ($url_info['http'] == 1) $protocol = 'http://';
						if ($url_info['https'] == 1) $protocol = 'https://';

						$old_url = $protocol.$tiny_url['url'].'/'.$state['asset']->id;
						$mapped_tiny_urls[$old_url] = $protocol.$url_info['url'];

						// If the asset has multiple webpaths, we just have to choose the first URL.
						break;
					}
				}

			}
		} else {
			return $result;
		}

		// Finally, insert Remap URLs
		if (!empty($mapped_tiny_urls)) {

			// Remove the previous remapped URLs already exist
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$old_urls = array_keys($mapped_tiny_urls);
			$sql = 'DELETE FROM
						'.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_remap
					WHERE url IN';

			$quoted_old_urls = Array();
			foreach ($old_urls as $old_url) {
				$quoted_old_urls[] = $db->quoteSmart($old_url);
			}
			$sql .= ' ('.implode(',', $quoted_old_urls).')';
			$result = $db->query($sql);
			assert_valid_db_result($result);

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			$remap_urls = $db->query($sql);
			assert_valid_db_result($remap_urls);

			$rm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
			foreach ($mapped_tiny_urls as $old_url => $new_url) {
				$result = $rm->addRemapURL($old_url, $new_url);
				switch ($result) {
					case 0:
						trigger_error('CORE0271', E_USER_WARNING);
					break;
					case -1:
						trigger_error('CORE0272', E_USER_WARNING, $old_url);
					break;
				}
			}
		}//end if

		$result['remap_urls'] = $mapped_tiny_urls;
		return $result;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		ob_start();
			echo translate('trigger_action_add_remap_message');
		return ob_get_clean();

	}//end getInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('lookups'));

	}//end getLocks()


}//end class

?>
