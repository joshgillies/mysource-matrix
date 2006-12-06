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
* $Id: site_edit_fns.inc,v 1.55 2006/12/06 02:59:56 emcdonald Exp $
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
* @version $Revision: 1.55 $
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


}//end class

?>
