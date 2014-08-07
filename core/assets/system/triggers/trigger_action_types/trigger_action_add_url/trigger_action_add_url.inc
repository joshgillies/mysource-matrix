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
* $Id: trigger_action_add_url.inc,v 1.18 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Add_URL
*
* A trigger action that adds a URL (with protocol option) to a site asset
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Add_Url extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*	'url' => string    // the web path to add
	*	'http' => boolean  // http protocol
	*	'https' => boolean // https protocol
	* );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$run_hipo = FALSE;
		// check required settings
		if (!isset($settings['url'])) return FALSE;

		// url not specified
		if ($settings['url'] == '') return FALSE;

		$settings['http'] = array_get_index($settings, 'http', 0);
		$settings['https'] = array_get_index($settings, 'https', 0);
		$settings['base_contextid'] = array_get_index($settings, 'base_contextid', 0);

		$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		// If the current setting doesn't exist, then revert to the default
		if (array_key_exists($settings['base_contextid'], $all_contexts) === FALSE) {
			$settings['base_contextid'] = 0;
		}

		// protocols not specified
		if (!$settings['http'] && !$settings['https']) {
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		if (is_null($state['asset'])) return FALSE;

		// if asset is not a site asset?
		if (!($state['asset'] instanceof Site)) {
			trigger_error('Unable to add URL for a '.$state['asset']->type().' asset', E_USER_NOTICE);
			return FALSE;
		}

		// dont change anything if the asset already has this URL
		$url_exists = FALSE;
		$urls = $state['asset']->getSiteURLs();
		foreach ($urls as $key => $value) {
			if ($value['url'] == $settings['url']) {
				if ($value['http'] == $settings['http'] && $value['https'] == $settings['https']) {
					$url_exists = TRUE;
					break;
				}
			}
		}

		if (!$url_exists) {
			// add a url

			$urls[0] = Array(
						'url'	=> $settings['url'],
						'http'	=> $settings['http'],
						'https'	=> $settings['https'],
						'base_contextid'	=> $settings['base_contextid'],
					   );

			// save a new url
			$old_urls = $state['asset']->getSiteURLs();
			switch ($state['asset']->saveWebURLs($urls)) {
				case 0:
					return FALSE;
				break;
				case 2:
					$run_hipo = TRUE;
				break;
			}



		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		foreach ($urls as $urlid => $url_data) {
			// Skip the new URL
			if (!isset($old_urls[$urlid]) || $old_urls[$urlid]==$url_data) {
				continue;
			}

			$run_hipo = TRUE;

			// update the root url in the asset url table to the new url
			try {
				$bind_vars	= Array (
								'url'			=> strip_url($urls[$urlid]['url']),
								'urlid'			=> $urlid,
							  );
				$result = MatrixDAL::executeQuery('site', 'updateUrl', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to update URL: '.strip_url($urls[$urlid]['url']).' for urlid: '.$urlid.' due to database error: '.$e->getMessage());
			}

			// update any urls that use this url in the lookup and lookup value tables
			foreach (Array('sq_ast_lookup_value', 'sq_ast_lookup') as $tablename) {
				// Skip if the url was not changed
				if ($urls[$urlid]['url'] == $old_urls[$urlid]['url']) {
					continue;
				}
				$sql = 'UPDATE
							'.$tablename.'
						SET
							url = :url || SUBSTR(url, :substr_arg)
						WHERE
						(url LIKE :old_url_like OR url LIKE :old_url_like_slash)';


						if ($tablename == 'sq_ast_lookup') {
							$sql .= ' AND root_urlid = :urlid';
						} else if ($tablename == 'sq_ast_lookup_value') {
							$sql .= ' AND url IN (
										SELECT
											l.url
										FROM
											sq_ast_lookup l
										INNER JOIN
											sq_ast_lookup_value v ON ((l.url = v.url) OR (l.url || \'/\' = v.url))
										WHERE
											l.root_urlid = :urlid
									)';
						}
				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'url', $urls[$urlid]['url']);
					MatrixDAL::bindValueToPdo($query, 'old_url_like', $old_urls[$urlid]['url'].'%');
					MatrixDAL::bindValueToPdo($query, 'old_url_like_slash', $old_urls[$urlid]['url'].'%/');
					MatrixDAL::bindValueToPdo($query, 'url', $urls[$urlid]['url']);
					MatrixDAL::bindValueToPdo($query, 'substr_arg', strlen($old_urls[$urlid]['url'])+1);
					MatrixDAL::bindValueToPdo($query, 'urlid', $urlid);
					$result = MatrixDAL::executePdoOne($query);
				} catch (Exception $e) {
					throw new Exception('Unable to update this old URL: '.$old_urls[$urlid]['url'].' with this new URL: '.$urls[$urlid]['url'].' due to database error: '.$e->getMessage());
				}
			}
		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		flush();

		}//end if


		if ($run_hipo) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetids' => Array($state['asset']->id));
			$hh->freestyleHipo('hipo_job_update_lookups', $vars);
		}//end if

		return Array(
				'assetid'	=> $state['asset']->id,
				'url'		=> $settings['url'],
				'http'		=> $settings['http'],
				'https'		=> $settings['https'],
				'base_contextid'	=> $settings['base_contextid'],
			   );

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
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		ob_start();

		$url = array_get_index($settings, 'url', '');
		$http = array_get_index($settings, 'http', '');
		$https = array_get_index($settings, 'https', '');
		$base_contextid = array_get_index($settings, 'base_contextid', 0);

		echo translate('add_url').' ';
		if ($write_access) {
			text_box($prefix.'[url]', $url, 40);
			echo '&nbsp;';
			check_box($prefix.'[http]', '1', $http);
			label('HTTP', $prefix.'[http]');
			echo '&nbsp;&nbsp;';
			check_box($prefix.'[https]', '1', $https);
			label('HTTPS', $prefix.'[https]');
			echo '&nbsp;&nbsp;';
		} else {
			echo '"<strong>'.$url.'</strong>" ';
		}

		$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		if (count($all_contexts) > 1) {
			// If the current setting doesn't exist, then revert to the default
			if (array_key_exists($base_contextid, $all_contexts) === FALSE) {
				$base_contextid = 0;
			}

			$options = Array();
			foreach ($all_contexts as $contextid => $context_data) {
				$options[$contextid] = $context_data['name'];
			}
			combo_box($prefix.'[base_contextid]', $options, FALSE, $base_contextid);
		}

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{

		// process url
		$url = array_get_index($request_data, 'url', '');
		$url = trim($url);
		if ($url == '') return translate('url_not_specified');
		$settings['url'] = $url;

		// process protocol
		$http = array_get_index($request_data, 'http', 0);
		$https = array_get_index($request_data, 'https', 0);
		$base_contextid = (int)array_get_index($request_data, 'base_contextid', 0);
		if (!$http && !$https) {
			return translate('protocol_not_specified');
		}

		$settings['http'] = ($http == '1') ? '1' : '0';
		$settings['https'] = ($https == '1') ? '1' : '0';

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('lookups'));

	}//end getLocks()


}//end class

?>
