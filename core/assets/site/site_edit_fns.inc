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
* $Id: site_edit_fns.inc,v 1.57 2006/12/06 04:57:30 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_edit_fns.inc';

/**
* Site_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.57 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Site_Edit_Fns extends Folder_Edit_Fns
{
	/**
	* All available protocols
	* @var Array(string)
	*/
	var $protos = Array('http', 'https');


	/**
	* Constructor
	*
	*/
	function Site_Edit_Fns()
	{
		$this->Folder_Edit_Fns();
		$this->static_screens['details']['lock_type'] = 'attr_links';
		unset($this->static_screens['dependants']);

	}//end constructor


	/**
	* Paint the interface for the asset urls
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintURLs(&$asset, &$o, $prefix)
	{
		?>
		<table border="0" cellspacing="0" cellpadding="3" id="urls_<?php echo $prefix; ?>-table">
			<tbody id="urls_<?php echo $prefix; ?>">
				<?php

				$urls = $asset->getSiteURLs();
				$wa = $asset->writeAccess('lookups');

				$urlids = Array();

				for (reset($urls); NULL !== ($urlid = key($urls)); next($urls)) {
					$this->_paintURL($o, $prefix, $urlid, $urls[$urlid], $wa);
					$urlids[] = $urlid;
				}//end for

				$last_id = 0;
				if (count($urlids) > 0) {
					$last_id = $urlids[count($urlids)-1];
				}

				if ($wa) {
					// paint a nice blank version
					$this->_paintURL($o, $prefix, ++$last_id, Array('url' => '', 'protocols' => Array()), TRUE);
					$urlids[] = $last_id;

					// let's be a bit nice about this, and when they submit confirm that they know what they are doing
					$o->addOnSubmit('if (!site_url_check()) return false;');

				}//end if


				?>
			</tbody>
		</table>
		<script language="JavaScript" type="text/javascript">

			var site_url_protos = new Array("<?php echo implode('","', $this->protos); ?>");
			var site_url_urlids = new Array("<?php echo implode('","', $urlids); ?>");

			var optionList = document.getElementById('urls_<?php echo $prefix; ?>');
			var	inputs = optionList.getElementsByTagName('TR');

			function expandList(el) {
				var lastId = site_url_urlids[site_url_urlids.length-1];
				lastId++;
				var lastInput = inputs[inputs.length-1];
				var newInput = lastInput.cloneNode(true);



				// get parent tr
				var parentTr = el.parentNode.parentNode;
				if (lastInput != parentTr || (inputs.length > 1 && inputs[inputs.length-2].getElementsByTagName('INPUT')[0].value == '')) {
					return;
				}

				newInput.getElementsByTagName('INPUT')[0].value = '';
				newInput.getElementsByTagName('INPUT')[0].id = '<?php echo $prefix; ?>' + '_urls['+lastId+'][url]';
				newInput.getElementsByTagName('INPUT')[0].name = '<?php echo $prefix; ?>' + '_urls['+lastId+'][url]';

				// protocols
				var prot = 0;
				for (var i = 1; i < newInput.getElementsByTagName('INPUT').length; i++) {
					newInput.getElementsByTagName('INPUT')[i].id = '<?php echo $prefix; ?>' + '_urls['+lastId+'][protocols]['+prot+']';
					newInput.getElementsByTagName('INPUT')[i].name = '<?php echo $prefix; ?>' + '_urls['+lastId+'][protocols]['+prot+']';
					prot++;
				}

				site_url_urlids.push(lastId);
				optionList.appendChild(newInput);

			}

			function site_url_check() {

				for (var j = 0; j < site_url_urlids.length; j++) {
					var urlid = site_url_urlids[j];

					// if there isn't any value in the url box, it doesn't matter
					var url = get_form_element_value("<?php echo $prefix; ?>_urls[" + urlid + "][url]");
					if (url.length) {
						var found = false;
						for (var i = 0; i < site_url_protos.length; i++) {
							if (document.getElementById("<?php echo $prefix; ?>_urls[" + urlid + "][protocols][" + i + "]").checked) {
								found = true;
								break;
							}
						}//end if

						// if nothing is checked, confirm deletion
						if (!found) {
							// if they don't want to delete, abort submit
							if (!confirm(js_translate('must_select_at_least_one_protocol', url))) {
								return false;
							}
						}

					// remove all the protocols
					} else {
						for (var i = 0; i < site_url_protos.length; i++) {
							var proto = get_form_element("<?php echo $prefix; ?>_urls[" + urlid + "][protocols][" + i + "]");
							proto.checked = false;
						}//end if

					}//end if

				}//end for

				return true;

			}//end site_url_check();

		</script>
		<?php

		return $wa;

	}//end paintURLs()


	/**
	* Paint the interface for individual URLs
	*
	* @param object		&$o				the Backend Outputter object
	* @param string		$prefix			prefix for the form element
	* @param int		$i				the count for this URL
	* @param array		$url_info		the urls info
	* @param boolean	$write_access	whether the user can edit this url
	*
	* @return boolean
	* @access public
	*/
	function _paintURL(&$o, $prefix, $i, $url_info, $write_access)
	{
		?>
			<tr>
				<td class="sq-backend-data">
				<?php
					if ($write_access) {
						text_box($prefix.'_urls['.$i.'][url]',  $url_info['url'], 40, '', FALSE, 'onFocus="expandList(this);"');
					} else {
						echo $url_info['url'];
					}
				?>
				</td>
		<?php
			for ($k = 0; $k < count($this->protos); $k++) {
			?>
				<td class="sq-backend-data">
				<?php
					if ($write_access) {
						check_box($prefix.'_urls['.$i.'][protocols]['.$k.']', $this->protos[$k], !empty($url_info[$this->protos[$k]]));
					} else {
					?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo empty($url_info[$this->protos[$k]]) ? 'cross' : 'tick'; ?>.gif" width="15" height="15" />
					<?php
					}
				?>
				</td>
				<td class="sq-backend-data">
					<?php echo strtoupper($this->protos[$k]); ?>
				</td>
			<?php
			}//end for
		?>
			</tr>
		<?php

	}//end _paintURL()


	/**
	* Processes the URLs posted from the paintURLs fn above
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processURLs(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_urls'])) return FALSE;

		$post = $_POST[$prefix.'_urls'];

		$urls = Array();
		for (reset($post); NULL !== ($urlid = key($post)); next($post)) {

			$url = trim($post[$urlid]['url']);
			// if there is a url or some selected protocols, then let's save it
			if ($url && !empty($post[$urlid]['protocols'])) {
				$urls[$urlid] = Array('url' => $post[$urlid]['url']);
				for ($k = 0; $k < count($this->protos); $k++) {
					$urls[$urlid][$this->protos[$k]] = (int) in_array($this->protos[$k], $post[$urlid]['protocols']);
				}
			}

		}

		$old_urls = $asset->getSiteURLs();
		if (!$asset->saveWebURLs($urls)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		foreach ($urls as $urlid => $url_data) {

			// Skip the new URL
			if (!isset($old_urls[$urlid])) continue;

			// update the root url in the asset url table to the new url
			$sql = 'UPDATE
						sq_ast_url
					SET
							url = '.$db->quoteSmart(strip_url($urls[$urlid]['url'])).'
					WHERE
							urlid = '.$db->quoteSmart($urlid);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			// update any urls that use this url in the lookup and lookup value tables
			foreach (Array('sq_ast_lookup_value', 'sq_ast_lookup') as $tablename) {

				// Skip if the url was not changed
				if ($urls[$urlid]['url'] == $old_urls[$urlid]['url']) continue;

				$sql = 'UPDATE
							'.$tablename.'
						SET
							url = '.$db->quoteSmart($urls[$urlid]['url']).' || SUBSTR(url, '.strlen($old_urls[$urlid]['url']).' + 1)
						WHERE
							url LIKE '.$db->quoteSmart($old_urls[$urlid]['url'].'%');

						if ($tablename == 'sq_ast_lookup') {
							$sql .= ' AND root_urlid = '.$db->quoteSmart($urlid);
						} else if ($tablename == 'sq_ast_lookup_value') {
							$sql .= 'AND url IN (
										SELECT
											l.url
										FROM
											sq_ast_lookup l
										INNER JOIN
											sq_ast_lookup_value v ON l.url = v.url
										WHERE
											l.root_urlid = '.$db->quoteSmart($urlid).'
									)';
						}

				$result = $db->query($sql);
				assert_valid_db_result($result);
			}
		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		flush();

		$run_hipo = TRUE;
/*
		// check the old URLs (if we've deleted one then we need to update the lookups)
		foreach ($old_urls as $old_urlid => $old_url_data) {
			if (!isset($urls[$old_urlid])) {
				$run_hipo = TRUE;
				break;
			}
		}

		// If we are adding a new URL, we need lookups added also
		if (isset($urls[0])) {
			$run_hipo = TRUE;
		}*/

		//// START HIPO PROCESSING ////
		if ($run_hipo) {
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetids' => Array($asset->id));
			$hh->queueHipo('hipo_job_update_lookups', $vars);
		}
		////  END HIPO PROCESSING  ////

		return TRUE;

	}//end processURLs()


	/**
	* Paint the interface for setting the index page for this site
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintIndexPage(&$asset, &$o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'index');

	}//end paintIndexPage()


	/**
	* Processes the setting of the index page for this site
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processIndexPage(&$asset, &$o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'index');

	}//end processIndexPage()


	/**
	* Paint the interface for setting the forbidden page for this site
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintForbiddenPage(&$asset, &$o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'forbidden');

	}//end paintForbiddenPage()


	/**
	* Processes the setting of the forbidden page for this site
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processForbiddenPage(&$asset, &$o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'forbidden');

	}//end processForbiddenPage()


	/**
	* Paint the interface for setting the not found page for this site
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintNotFoundPage(&$asset, &$o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'not_found');

	}//end paintNotFoundPage()


	/**
	* Processes the setting of the not found page for this site
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNotFoundPage(&$asset, &$o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'not_found');

	}//end processNotFoundPage()


	/**
	* Paint the interface for setting authentication redirects for the site
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAuthURLs(&$asset, &$o, $prefix)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$write_access = $asset->writeAccess('lookups');

		$sql = 'SELECT
					urlid, url, auth_urlid
				FROM
					sq_ast_url
				WHERE
					assetid = '.$db->quote($asset->id);

		$urls = $db->getAssoc($sql);
		assert_valid_db_result($urls);

		foreach ($urls as $urlid => $url) {
			$combo_box_urls[$urlid] = $url['url'];
		}

		$combo_box_from = $combo_box_urls;

		// We will cache the body of the table, and only print the table out
		// if we have a redirect worth painting
		$paint_table = FALSE;

		ob_start();

			foreach ($urls as $urlid => $url) {
				if (!empty($url['auth_urlid'])) {
					$paint_table = TRUE;
					unset($combo_box_from[$urlid]);
					?>
					<tr>
					<td><?php echo $url['url'] ?></td>
					<td><?php
					if ($write_access) {
						combo_box($prefix.'auth_url[change]['.$urlid.']', $combo_box_urls, FALSE, $url['auth_urlid']);
					} else {
						echo $urls[$url['auth_urlid']]['url'];
					}
					?></td>
					<?php if ($write_access) {
					?>
					<td><?php echo check_box($prefix.'auth_url[delete]['.$urlid.']') ?></td>
					<?php
					}
					?>
					</tr>

					<?php
				}
			}

		$body = ob_get_clean();

		$o->openSection(translate('core_site_current_redirects'));
		$o->openField('');

		if ($paint_table) {
			?>
			<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('old_url') ?></th>
				<th><?php echo translate('new_url') ?></th>
				<?php if ($write_access) {
					?><th>Delete?</th><?php
				}
				?>
			</tr>
			<?php
			echo $body;
			?>
			</table>
			<?php
		} else {
			?><em>No redirects currently exist</em><?php
		}

		$o->closeField();
		$o->closeSection();

		if ($write_access) {

			// Now the "add" section
			$combo_box_from = Array('' => '-- Select URL --') + $combo_box_from;
			$combo_box_urls = Array('' => '-- Select URL --') + $combo_box_urls;

			$o->openSection(translate('core_site_add_redirect'));

				$o->openField(translate('from'));
					combo_box($prefix.'auth_url[from]', $combo_box_from);
				$o->closeField();

				$o->openField(translate('to'));
					combo_box($prefix.'auth_url[to]', $combo_box_urls);
				$o->closeField();

			$o->closeSection();

		}

		return $write_access;

	}//end paintAuthURLs()


	/**
	* Process the interface for setting authentication redirects for the site
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAuthURLs(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('lookups')) return FALSE;

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT
					urlid, auth_urlid
				FROM
					sq_ast_url
				WHERE
					assetid = '.$db->quote($asset->id).';';

		$urls = $db->getAssoc($sql);
		assert_valid_db_result($urls);
		$changed_urlids = Array();

		$auth_urls = $_REQUEST[$prefix.'auth_url'];

		// All this includes security checks to make sure the passed request
		// data are at least well-formed
		if (!is_array($auth_urls)) return FALSE;

		if (isset($auth_urls['change'])) {
			if (!is_array($auth_urls['change'])) return FALSE;

			foreach ($auth_urls['change'] as $from => $to) {
				if (!is_numeric($to)) return FALSE;
				$urls[$from] = (int)$to;
				$changed_urlids[$from] = TRUE;
			}
		}

		if (isset($auth_urls['delete'])) {
			if (!is_array($auth_urls['delete'])) return FALSE;

			foreach ($auth_urls['delete'] as $from => $value) {
				$urls[$from] = NULL;
				$changed_urlids[$from] = TRUE;
			}
		}

		if (!empty($auth_urls['from']) && !empty($auth_urls['to'])) {
			if (!is_numeric($auth_urls['from'])) return FALSE;
			if (!is_numeric($auth_urls['to'])) return FALSE;

			$urls[(int)$auth_urls['from']] = (int)$auth_urls['to'];
			$changed_urlids[(int)$auth_urls['from']] = TRUE;
		}

		if ($this->_isCircularReference($urls)) {
			// Cannot update private URLs, the submitted set-up would cause a
			// redirect loop
			trigger_localised_error('CORE0263', E_USER_WARNING);
			return FALSE;
		} else {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// save them
			foreach (array_keys($changed_urlids) as $urlid) {
				$sql = 'UPDATE
							sq_ast_url
						SET
							auth_urlid = '.$db->quote($urls[$urlid]).'
						WHERE
							urlid = '.$db->quote($urlid).';';
				$result = $db->query($sql);
				assert_valid_db_result($result);
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		}

	}//end processAuthURLs()


	/**
	* Checks the URL list to see if there is a circular reference that would
	* cause a redirect loop
	*
	* Algorithm:
	* - Start on each from URL ID, traverse to the to URL ID, and that becomes
	*   the next "from".
	* - If at any point we hit a "no redirect", then there is no loop with that
	*   particular URL.
	* - If at any point we hit a URL ID that we have seen before from a
	*   particular, then we have looped, so return TRUE. (Obviously we start
	*   afresh from each starting point.)
	*
	* Example:
	*     Example URL setup              Circular?
	*     ============================== =========
	*     A => B, B => A                   YES
	*     A => B, B => C, C => A           YES
	*     A => B, B => C, C => NULL         NO
	*
	* @param array	$urls	array of URL IDs
	*
	* @return boolean
	* @access private
	*/
	function _isCircularReference($urls)
	{
		foreach (array_keys($urls) as $from) {
			$current_from = $from;
			$seen = Array();
			$complete = FALSE;
			$circular = FALSE;

			while (!$complete && !$circular) {
				$seen[] = $current_from;

				// Get the next step in the list
				$current_from = array_get_index($urls, $current_from, NULL);

				if (is_null($current_from)) {
					$complete = TRUE;
				}

				// Have we already seen this

				if (array_search($current_from, $seen) !== FALSE) {
					$circular = TRUE;
				}

			}//end while

			// We found a circular reference
			if ($circular) return TRUE;

		}//end foreach

		// No circular reference detected
		return FALSE;

	}//end _isCircularReference()


}//end class

?>
