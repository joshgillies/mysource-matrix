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
* $Id: insert_image_search.php,v 1.8 2013/04/23 08:08:32 cupreti Exp $
*
*/

/**
* Insert Link Popup for the WYSIWYG
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_LIB_PATH.'/backend_search/backend_search.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

if (Backend_Search::isAvailable()) {
	$quick_search_for_text = translate('asset_search_default_keyword');
} else {
	$quick_search_for_text = translate('asset_search_default');
}

$search_for = trim(array_get_index($_GET, 'quick-search-for', ''));

// If we are searching for something
if ($search_for != '') {
	// check for a url first
	$asset_by_url = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL('', strip_url($search_for, TRUE), TRUE, TRUE);
	if (!is_a($asset_by_url, 'image') || !$asset_by_url->readAccess()) {
		$asset_by_url = NULL;
	}

	if (assert_valid_assetid($search_for, '', TRUE, FALSE)) {
		$asset_by_id  = $GLOBALS['SQ_SYSTEM']->am->getAsset($search_for, '', TRUE);
		if (!is_a($asset_by_id, 'image') || !$asset_by_id->readAccess()) {
			$asset_by_id = NULL;
		}
	}

	$html = '';
	$found_asset_line = '';
	$results = Array();

	if (!empty($asset_by_url)) {
		$found_asset =& $asset_by_url;
		$found_asset_line .= '<strong>'.'Matched on URL:'.'</strong>';
	}

	if (!empty($asset_by_id)) {
		$found_asset =& $asset_by_id;
		$found_asset_line .= '<strong>'.'Matched on Asset ID:'.'</strong>';
	}

	if (!empty($found_asset)) {
		$asset_title = $found_asset->attr('title');
		$asset_alt = addslashes($found_asset->attr('alt'));
		$asset_width = $found_asset->attr('width');
		$asset_height = $found_asset->attr('height');
		$asset_name = $found_asset->name;
		$found_asset_line .= '<div class="search-result">';
		$found_asset_line .= get_asset_tag_line($found_asset->id, 'javascript:set_asset_finder_from_search(\''.$found_asset->id.'\', \''.htmlspecialchars($asset_title, ENT_QUOTES).'\', \'\', \'0\', \''.$asset_name.'\', \''.$asset_alt.'\', \''.$asset_width.'\', \''.$asset_height.'\');');
		$found_asset_line .= '</div>';
	} else {
		// Only search for images
		$results = Backend_Search::processSearch($search_for, Array(), Array('image' => TRUE));

		if (!empty($results)) {
			$result_list = Array();

			foreach ($results as $result_assetid => $result_detail) {
				$tag_line = get_asset_tag_line($result_assetid);

				$this_detail = Array();

				foreach ($result_detail as $result_component_name => $result_component) {
					foreach ($result_component as $name => $value) {

						$name_detail = '';
						switch ($result_component_name) {
							case 'contents':
								$name_detail = 'Asset Contents';
							break;

							case 'metadata':
							case 'schema':
								$name_detail = ($result_component_name == 'schema' ? 'Default ' : '').'Metadata: ';

								// Find a friendly name for the metadata field, if there
								// is none then use the standard name of the field itself
								$attr_values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('friendly_name', 'metadata_field', Array($name));
								if (empty($attr_values)) {
									$name_detail .= $value['name'];
								} else {
									$name_detail .= $attr_values[$name];
								}

								$value = $value['value'];
							break;

							case 'attributes':
								$name_detail = 'Attribute: '.ucwords(str_replace('_', ' ', $name));
							break;
						}

						$words = explode(' ', $search_for);
						$value = strip_tags($value);

						preg_match_all('/('.addslashes(implode('|', $words)).')/i', $value, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE);

						// We go backwards, because that way we don't invalidate
						// our offsets. This section ellipsisises the bits between
						// matches so that there's 15 characters either side of
						// matches.

						// Last match position
						if ($matches[count($matches) - 1][0][1] < strlen($value) - 15) {
							$value = substr_replace($value, '...', $matches[count($matches) - 1][0][1] + 15);
						}

						for ($i = count($matches) - 1; $i > 0; $i--) {
							$previous_match = $matches[$i - 1][0];
							$this_match = $matches[$i][0];

							$prev_pos = $previous_match[1] + strlen($previous_match[0]);
							$next_pos = $this_match[1];

							if (($next_pos - $prev_pos) > 30) {
								$value = substr_replace($value, '...', $prev_pos + 15, ($next_pos - $prev_pos) - 30);
							}
						}

						// First match position
						if ($matches[0][0][1] > 15) {
							$value = substr_replace($value, '...', 0, $matches[0][0][1] - 15);
						}

						// Cut it down to a certain number of characters anyway
						$value = ellipsisize($value, 120);

						$value = preg_replace('/('.addslashes(implode('|', $words)).')/i', '<span class="sq-backend-search-results-highlight">$1</span>', $value);

						// remove \r and replace \n with line breaks
						$this_detail[] = $name_detail.'<br/><em>'.str_replace("\r", '', str_replace("\n", '<br/>', $value)).'</em>';
					}//end foreach
				}//end foreach

				$asset_name = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($result_assetid), 'image', FALSE, 'name');
				$asset_title = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('title', 'image', Array($result_assetid));
				$asset_alt = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('alt', 'image', Array($result_assetid));
				$asset_width = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('width', 'image', Array($result_assetid));
				$asset_height = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('height', 'image', Array($result_assetid));

				$result_list[] = Array(
									'tag_line'	=> get_asset_tag_line($result_assetid, 'javascript:set_asset_finder_from_search(\''.$result_assetid.'\', \''.htmlspecialchars(array_get_index($asset_title, $result_assetid, ''), ENT_QUOTES).'\', \'\', \'0\', \''.array_get_index($asset_name, $result_assetid, '').'\', \''.addslashes(array_get_index($asset_alt, $result_assetid, '')).'\', \''.array_get_index($asset_width, $result_assetid, '').'\', \''.array_get_index($asset_height, $result_assetid, '').'\');'),
									'detail'	=> implode($this_detail, '<br/>'),
								 );
			}//end foreach
		}//end if
	}//end else

	// Are there any results? If not, put in a "search failed" box, otherwise
	// build the results box
	if (empty($results) && empty($found_asset_line)) {
		$box_title = translate('search_failed');
		$html = translate('failed_searching_for', addslashes($search_for));
		$style_base = 'search-failed';
	} else {
		$box_title = translate('search_results');

		if (!empty($found_asset_line)) {
			$html .= '<div class="search-result">'.$found_asset_line.'</div>';
		}

		if (count($results) > 0) {

			$result_number = 0;
			$results_per_page = $GLOBALS['SQ_SYSTEM']->getUserPrefs('search_manager', 'SQ_SEARCH_BACKEND_PAGE_SIZE');
			$total_pages = ceil(count($results) / $results_per_page);

			$html .= '<div class="search-result-blurb">Matched on Keyword ('.count($results).' asset'.(count($results) == 1 ? '' : 's').'):</div>';

			if ($total_pages > 1) {
				$html .= '<div class="search-result-pager">
							<a href="" onclick="jump_to_search_results(1); return false;" title="Go back to first page">&lt;&lt;</a> &nbsp;
							<a href="" onclick="jump_to_search_results(Math.max(current - 1, 1)); return false;" title="Go back one page">&lt;</a> &nbsp;
							<strong>( <span id="sq-search-results-page-start">1</span> - <span id="sq-search-results-page-end">'.min(count($results), $results_per_page).'</span> )</strong> &nbsp;
							<a href="" onclick="jump_to_search_results(Math.min(current + 1, '.$total_pages.')); return false;" title="Go forward one page">&gt;</a> &nbsp;
							<a href="" onclick="jump_to_search_results('.$total_pages.'); return false;" title="Go forward to last page">&gt;&gt;</a>
							</div>';
			}

			foreach ($result_list as $this_result) {
				$page_number = floor($result_number / $results_per_page) + 1;

				// Start of a new page
				if ($result_number % $results_per_page == 0) {
					if ($page_number > 1) $html .= '</div>';
					$html .= '<div class="search-result-page" id="search-result-page-'.$page_number.'"';
					if ($page_number > 1) {
						$html .= ' style="display: none"';
					}
					$html .= '>';
				}

				$result_number++;

				$html .= '<div class="search-result" id="search-result-'.$result_number.'">';
				$html .= '<div class="search-result-expand-div" id="search-result-'.$result_number.'-expand-div">';
				$html .= '<a href="#" id="search-result-'.$result_number.'-expand-link" class="search-result-expand-link" onclick="if (this.innerHTML == \'+\') {document.getElementById(\'search-result-'.$result_number.'-detail\').style.display = \'block\'; this.innerHTML = \'-\';} else {document.getElementById(\'search-result-'.$result_number.'-detail\').style.display = \'none\'; this.innerHTML = \'+\';} return false;">+</a>';
				$html .= '</div>';
				$html .= '<div class="search-result-entry" id="search-result-'.$result_number.'-entry">'.$this_result['tag_line'].'</div>';
				$html .= '<div class="search-result-detail" id="search-result-'.$result_number.'-detail">'.$this_result['detail'].'</div>';
				$html .= '</div> ';
			}

			// End of last page
			$html .= '</div>';

		}//end if

		$style_base = 'search-results';
	}//end else

	$html = str_replace("\r", '', $html);
	$html = str_replace("\n", '  ', $html);

}//end if - we are searching for something
?>

<html>
	<head>
		<title>Insert Image - Search</title>
		<style type="text/css">
			@import url("<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css");
		</style>
		<script type="text/javascript"><!--
			/**
			* Run when quick-search-for box is tabbed/clicked to
			*
			* @param object	field	reference to the text box
			*
			* @return boolean
			* @access public
			*/
			function quick_search_for_onfocus(field)
			{
				if (field.value == '<?php echo addslashes($quick_search_for_text) ?>') {
					field.value = '';
				}

				return true;
			}


			/**
			* Run when quick-search-for box is tabbed/clicked away from
			*
			* If we click away from the field without filling in anything, return
			* to the
			*
			* @param object	field	reference to the text box
			*
			* @return boolean
			* @access public
			*/
			function quick_search_for_onblur(field)
			{
				if (field.value == '') {
					field.value = '<?php echo addslashes($quick_search_for_text) ?>';
				}

				return true;
			}


			/**
			* Run when quick-search box is submitted
			*
			* @param object	form	reference to the form itself
			*
			* @return boolean
			* @access public
			*/
			function quick_search_onsubmit(form)
			{
				if ((form.elements['quick-search-for'].value == '') || (form.elements['quick-search-for'].value == '<?php echo addslashes($quick_search_for_text) ?>')) {
					return false;
				}

				top.frames['sq_wysiwyg_popup_main'].document.getElementById('search-wait-popup').style.display = 'block';
				top.frames['sq_wysiwyg_popup_main'].document.getElementById('new-message-popup').style.display = 'none';

				return true;
			}
<?php
// If something was being searched for, fill and show the search results
if ($search_for != '') {
?>
			var counter = 0;
			var interval;

			// Reset variables in the main frame
			top.frames['sq_wysiwyg_popup_main'].total_results = <?php echo count($results); ?>;
			top.frames['sq_wysiwyg_popup_main'].current = 1;

			function show_search_results()
			{
				top.frames['sq_wysiwyg_popup_main'].document.getElementById('search-wait-popup').style.display = 'none';

				var results_box = top.frames['sq_wysiwyg_popup_main'].document.getElementById('new-message-popup');
				var results_box_text = top.frames['sq_wysiwyg_popup_main'].document.getElementById('new-message-popup-details');
				var results_box_title = top.frames['sq_wysiwyg_popup_main'].document.getElementById('new-message-popup-title');
				var results_box_titlebar = top.frames['sq_wysiwyg_popup_main'].document.getElementById('new-message-popup-titlebar');

				var html = '<?php echo addslashes($html) ?>';
				var title_html = '<?php echo addslashes($box_title) ?>';
				var class_name = '<?php echo addslashes($style_base) ?>';

				results_box.className = 'sq-backend-' + class_name + '-table';
				results_box_titlebar.className = 'sq-backend-' + class_name + '-heading';
				results_box_text.className = 'sq-backend-' + class_name + '-body';

				results_box.style.visibility = 'hidden';

				results_box_title.innerHTML = title_html;
				results_box_text.innerHTML = html;
				results_box.style.display = 'block';

				height = results_box.offsetHeight;
				counter = height;
				results_box.style.top = -counter + 'px';

				results_box.style.visibility = 'visible';

				interval = setInterval('move_search_results();', 30);
			}

			function move_search_results()
			{
				move = Math.max(Math.min(Math.abs(counter) * 0.08, 10), 2);
				counter -= move;

				var results_box = top.frames['sq_wysiwyg_popup_main'].document.getElementById('new-message-popup');
				results_box.style.top = -counter + 'px';

				if (counter <= 0) {
					clearTimeout(interval);
				}
			}
<?php
}//end if - something was being searched for
?>
		// --></script>
	</head>

	<body<?php if ($search_for != '') {
	?>
	 onload="show_search_results();"
	<?php
	}
	?>>
		<?php

		?>
		<div class="title"><form action="" method="get" id="quick-search" onsubmit="return quick_search_onsubmit(this);">
			<label id="quick-search-for-label" for="quick-search-for">Quick Search for Images</label>
			<input type="text" size="30" value="<?php echo htmlspecialchars(empty($search_for) ? $quick_search_for_text : $search_for); ?>" name="quick-search-for" id="quick-search-for" onfocus="return quick_search_for_onfocus(this);" onblur="return quick_search_for_onblur(this);" onkeypress="if (event.keyCode == 13) return quick_search_onsubmit(this.form);">
		</form>
		</div>
	</body>
</html>
