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
* $Id: site_edit_fns.inc,v 1.84 2013/07/25 23:23:48 lwright Exp $
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
* @version $Revision: 1.84 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Site_Edit_Fns extends Folder_Edit_Fns
{
	/**
	* All available protocols
	* @var Array(string)
	*/
	protected $protos = Array('http', 'https');


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['lock_type'] = 'attr_links';
		unset($this->static_screens['dependants']);

	}//end constructor


	/**
	* Paint the interface for the asset urls
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintURLs(Asset &$asset, Backend_Outputter &$o, $prefix)
	{
		?>
		<table id="urls_<?php echo $prefix; ?>-table" class="sq-backend-table" style="width: auto">
			<thead>
				<tr>
					<th>URL</th>
					<?php
						foreach ($this->protos as $protocol) {
							?><th><?php echo strtoupper($protocol) ?></th><?php
						}

						$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
						if (count($all_contexts) > 1) {
							?><th>Base Context</th><?php
						}
					?>
				</tr>
			</thead>
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

				// Base context, if it's there
				if (newInput.getElementsByTagName('SELECT').length > 0) {
					newInput.getElementsByTagName('SELECT')[0].value = 0;
					newInput.getElementsByTagName('SELECT')[0].id = '<?php echo $prefix; ?>' + '_urls['+lastId+'][base_contextid]';
					newInput.getElementsByTagName('SELECT')[0].name = '<?php echo $prefix; ?>' + '_urls['+lastId+'][base_contextid]';
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
							if (!confirm(js_translate('You need to select at least one protocol, otherwise the URL will be deleted. Press OK to remove the URL, or Cancel to abort and select a protocol.', url))) {
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
	* @param Backend_Outputter	$o				the Backend Outputter object
	* @param string				$prefix			prefix for the form element
	* @param int				$i				the count for this URL
	* @param array				$url_info		the urls info
	* @param boolean			$write_access	whether the user can edit this url
	*
	* @return boolean
	* @access protected
	*/
	protected function _paintURL(Backend_Outputter $o, $prefix, $i, Array $url_info, $write_access)
	{
		$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		?>
			<tr>
				<td class="sq-backend-table-cell">
				<?php
					if ($write_access) {
						text_box($prefix.'_urls['.$i.'][url]',  $url_info['url'], 40, '', FALSE, 'onFocus="expandList(this);"');
					} else {
						echo '<strong><a href="'.(empty($url_info[$this->protos[0]]) ? 'https://' : 'http://').$url_info['url'].'" target="_blank">'.$url_info['url'].'</a></strong>';
					}
				?>
				</td>
		<?php
			for ($k = 0; $k < count($this->protos); $k++) {
			?>
				<td class="sq-backend-table-cell" style="text-align: center">
				<?php
					if ($write_access) {
						check_box($prefix.'_urls['.$i.'][protocols]['.$k.']', $this->protos[$k], !empty($url_info[$this->protos[$k]]));
					} else {
					?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo empty($url_info[$this->protos[$k]]) ? 'cross' : 'tick'; ?>.png" width="15" height="15" />
					<?php
					}
				?>
				</td>
			<?php
			}//end for

			if (count($all_contexts) > 1) {
				?><td class="sq-backend-table-cell"><?php
				// If the base context hasn't been saved yet, use the default
				if (array_key_exists('base_contextid', $url_info) === FALSE) {
					$url_info['base_contextid'] = 0;
				}

				// If the context no longer exists, revert back to default
				if (array_key_exists($url_info['base_contextid'], $all_contexts) === FALSE) {
					$url_info['base_contextid'] = 0;
				}

				if ($write_access === TRUE) {
					$options = Array();
					foreach ($all_contexts as $contextid => $context_data) {
						$options[$contextid] = $context_data['name'];
					}
					combo_box($prefix.'_urls['.$i.'][base_contextid]', $options, FALSE, $url_info['base_contextid']);
				} else {
					echo $all_contexts[$url_info['base_contextid']]['name'];
				}
			?></td><?php
			}//end if multiple contexts exist
		?>
		</tr>
		<?php

	}//end _paintURL()


	/**
	* Processes the URLs posted from the paintURLs fn above
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the Backend Outputter object
	* @param string		$prefix			prefix for the form element
	* @param boolean	$updatelookups	TRUE = run HIPO job as normal, FALSE = don't run HIPO job
	*
	* @return boolean
	* @access public
	*/
	public function processURLs(Site $asset, Backend_Outputter $o, $prefix, $updatelookups=TRUE)
	{
		if (empty($_POST[$prefix.'_urls'])) return FALSE;

		$post = $_POST[$prefix.'_urls'];

		$urls = Array();
		for (reset($post); NULL !== ($urlid = key($post)); next($post)) {

			$url = preg_replace('/^http(s)?:\/\//', '', trim($post[$urlid]['url']));

			// if there is a url or some selected protocols, then let's save it
			if ($url && !empty($post[$urlid]['protocols'])) {
				$urls[$urlid] = Array(
									'url'				=> $url,
									'base_contextid'	=> array_get_index($post[$urlid], 'base_contextid', 0),
								);
				for ($k = 0; $k < count($this->protos); $k++) {
					$urls[$urlid][$this->protos[$k]] = (int) in_array($this->protos[$k], $post[$urlid]['protocols']);
				}
			}

		}
		$run_hipo = FALSE;
		$old_urls = $asset->getSiteURLs();
		switch ($asset->saveWebURLs($urls)) {
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
											v.url
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
					MatrixDAL::bindValueToPdo($query, 'url',                $urls[$urlid]['url']);
					MatrixDAL::bindValueToPdo($query, 'old_url_like',       $old_urls[$urlid]['url'].'%');
					MatrixDAL::bindValueToPdo($query, 'old_url_like_slash', $old_urls[$urlid]['url'].'%/');
					MatrixDAL::bindValueToPdo($query, 'substr_arg',         strlen($old_urls[$urlid]['url'])+1);
					MatrixDAL::bindValueToPdo($query, 'urlid',              $urlid);
					$result = MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to update this old URL: '.$old_urls[$urlid]['url'].' with this new URL: '.$urls[$urlid]['url'].' due to database error: '.$e->getMessage());
				}
			}
		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		flush();

		//// START HIPO PROCESSING ////
		if ($run_hipo && $updatelookups) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetids' => Array($asset->id));
			$hh->queueHipo('hipo_job_update_lookups', $vars);
		}
		////  END HIPO PROCESSING  ////

		return TRUE;

	}//end processURLs()


	/**
	* Paint the interface for setting the index page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintIndexPage(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'index');

	}//end paintIndexPage()


	/**
	* Processes the setting of the index page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processIndexPage(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'index');

	}//end processIndexPage()


	/**
	* Paint the interface for setting the "archived 404" page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintArchivedPage(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'archive');

	}//end paintIndexPage()


	/**
	* Processes the setting of the "archived 404" page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processArchivedPage(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'archive');

	}//end processIndexPage()


	/**
	* Paint the interface for setting the forbidden page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintForbiddenPage(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'forbidden');

	}//end paintForbiddenPage()


	/**
	* Processes the setting of the forbidden page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processForbiddenPage(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'forbidden');

	}//end processForbiddenPage()


	/**
	* Paint the interface for setting the not found page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintNotFoundPage(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'not_found');

	}//end paintNotFoundPage()


	/**
	* Processes the setting of the not found page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processNotFoundPage(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'not_found');

	}//end processNotFoundPage()


	/**
	* Paint the interface for the design setting the not found page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintNotFoundPageDesign(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'not_found_design', Array('design' => 'D'));

	}//end paintNotFoundPageDesign()


	/**
	* Processes the design setting of the not found page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processNotFoundPageDesign(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'not_found_design');

	}//end processNotFoundPageDesign()


	/**
	* Paint the interface for the paint layout setting the not found page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintNotFoundPagePaintLayout(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'not_found_layout', Array('paint_layout_page' => 'D'));

	}//end paintNotFoundPagePaintLayout()


	/**
	* Processes the design setting of the not found page for this site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processNotFoundPagePaintLayout(Site $asset, Backend_Outputter $o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'not_found_layout');

	}//end processNotFoundPagePaintLayout()


	/**
	* Paint the interface for setting authentication redirects for the site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintAuthURLs(Site $asset, Backend_Outputter $o, $prefix)
	{
		$db = MatrixDAL::getDb();
		$write_access = $asset->writeAccess('lookups');

		try {
			$bind_vars = Array('assetid' => $asset->id);
			$urls = MatrixDAL::executeAssoc('site', 'getUrlsFromAssetId', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get URLs for asset: '.$asset->id.' due to database error: '.$e->getMessage());
		}

		$combo_box_urls = Array();

		foreach ($urls as $urlid => $url) {
			$combo_box_urls[$url['urlid']] = $url['url'];
		}

		$combo_box_from = $combo_box_urls;

		// We will cache the body of the table, and only print the table out
		// if we have a redirect worth painting
		$paint_table = FALSE;

		ob_start();

			foreach ($urls as $urlid => $url) {
				if (!empty($url['auth_urlid'])) {
					$paint_table = TRUE;
					$auth_urlid = $url['auth_urlid'];
					unset($combo_box_from[$urlid]);
					?>
					<tr>
					<td><?php echo $url['url'] ?></td>
					<td><?php
					if ($write_access) {
						combo_box($prefix.'auth_url[change]['.$url['urlid'].']', $combo_box_urls, FALSE, $url['auth_urlid']);
					} else {
						for ($i=0; $i<count($urls); $i++) {
							if ($urls[$i]['urlid'] === $auth_urlid)
								echo $urls[$i]['url'];
						}
					}
					?></td>
					<?php if ($write_access) {
					?>
					<td><?php echo check_box($prefix.'auth_url[delete]['.$url['urlid'].']') ?></td>
					<?php
					}
					?>
					</tr>

					<?php
				}
			}

		$body = ob_get_clean();

		$o->openSection(translate('Current Redirects'));

		$o->openField('');

		if ($paint_table) {
			?>
			<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Old URL') ?></th>

				<th><?php echo translate('New URL') ?></th>

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

			$o->openSection(translate('Add Redirect'));


				$o->openField(translate('From'));

					combo_box($prefix.'auth_url[from]', $combo_box_from);
				$o->closeField();

				$o->openField(translate('To'));

					combo_box($prefix.'auth_url[to]', $combo_box_urls);
				$o->closeField();

			$o->closeSection();

		}

		return $write_access;

	}//end paintAuthURLs()


	/**
	* Process the interface for setting authentication redirects for the site
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processAuthURLs(Site $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('lookups')) return FALSE;

		$db = MatrixDAL::getDb();

		try {
			$bind_vars = Array('assetid' => $asset->id);
			$urls = MatrixDAL::executeAssoc('site', 'getUrlsFromAssetId', $bind_vars);
			// transform
			$transformed_urls = Array();
			foreach ($urls as $key => $value) {
				if (isset($value['urlid'])) {
					$transformed_urls[$value['urlid']] = $value['auth_urlid'];
				}
			}
			$urls = $transformed_urls;

		} catch (Exception $e) {
			throw new Exception('Unable to get authentication redirect URLs for asset: '.$asset->id.' due to database error: '.$e->getMessage());
		}
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
			trigger_localised_error('CORE0269', translate('Could not update authentication redirects, the set-up submitted would cause a redirect loop'), E_USER_WARNING);
			return FALSE;
		} else {
			// determine if there are any new or deleted urls, we are only interested in the deleted ones
			if (isset($_POST[$prefix.'_urls'])) {
				$processed_urls = $_POST[$prefix.'_urls'];
				$current_urls = $asset->getSiteURLs();
				$diff_urls = array_values(array_diff(array_keys($processed_urls), array_keys($current_urls)));
			}

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db = MatrixDAL::getDb();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// save them
			foreach (array_keys($changed_urlids) as $urlid) {
				if (isset($diff_urls) && !in_array($urls[$urlid], $diff_urls)) {
					try {
						$bind_vars	= Array (
										'auth_urlid'	=> $urls[$urlid],
										'urlid'			=> $urlid,
									  );
						$results = MatrixDAL::executeQuery('site', 'updateAuthUrlId', $bind_vars);
					} catch (Exception $e) {
						throw new Exception('Unable to update auth_urlid for urlid: '.$urlid.' due to database error: '.$e->getMessage());
					}
				}
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
	*-
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
	protected function _isCircularReference(Array $urls)
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

				if (empty($current_from)) $complete = TRUE;

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
