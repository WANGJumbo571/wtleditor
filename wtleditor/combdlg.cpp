#include "stdafx.h"
#include "combdlg.h"
#include "mainfrm.h"
#include "ImageSVGOperation.h"

extern CMainFrame* g_frameWork;

static CListViewCtrl	m_wndListView;
static CImageList		m_imgList;

static WNDPROC OldWndProc = NULL;
static CombDlg* combDlg_ptr = NULL;

#define COMBDLG_BUFLEN 100
#define SHOWPIXELS 150

int CombDlg::GetStopCharLoc(WCHAR* namebuf, int len, HDC hdc)
{
	WCHAR* buf;
	buf = (WCHAR*)malloc(sizeof(WCHAR) * wcslen(namebuf) + 2);

	for (int i = 0; i < (int)wcslen(namebuf); i++)
	{
		SIZE size;
		wcscpy(buf, namebuf);
		buf[min(COMBDLG_BUFLEN, i)] = L'\0';
		GetTextExtentPoint32(hdc, buf, min(COMBDLG_BUFLEN, i), &size);
		if (size.cx >= len)
		{
			free(buf);
			return min(COMBDLG_BUFLEN, i);
		}
	}
	free(buf);
	return min(wcslen(namebuf), COMBDLG_BUFLEN);
}

//---------------------------------------------------------------------------------------------------------------------------------
// ComboBox 的各种局部数据和操作例程。
//---------------------------------------------------------------------------------------------------------------------------------
#define ZEROPLACE												((int)layer_Index.size()-1)

static std::vector<int>											layer_Index;
static std::vector<CString>									layer_titles;
static std::vector<ComPtr<IImageOperation> >	layer_items;

static WCHAR namebuf[COMBDLG_BUFLEN + 1];
static bool fontInitial = false;
static CFont fontSmall;
static LOGFONT logFontSmall;

//---------------------------------------------------------------------------------------------------------------------------------
// 队列里至少有一个空白元素（在队列末尾）。
//---------------------------------------------------------------------------------------------------------------------------------
bool CombDlg::COMB_HasListMember()
{
	return ((int)layer_Index.size() > 1);
}

HBITMAP CombDlg::COMB_MakeHBITMAP(WCHAR* filename)
{
	ComPtr<IWICBitmapFlipRotator> wicBitmapFRer;
	ComPtr<IWICBitmapSource> source;

	g_pWICFactory->CreateBitmapFlipRotator(&wicBitmapFRer);
	source = g_render->LoadWicBitmapFromFile(filename, 36, 36);
	wicBitmapFRer->Initialize(source, WICBitmapTransformFlipVertical);
	return Direct2DUtility::CreateHBITMAP(wicBitmapFRer);
}

//---------------------------------------------------------------------------------------------------------------------------------
// COMB_InitList 从 Operations 队列里过滤出可显示的 operation（Text, Pic, Geo） 并把它们的信息初始化在
// layer_Index，layer_titles 和 layer_items 里，并在三个队列的最后一项上添加 Blank 元素，最后选择这个空白
// 元素做 ComboBox 的显示。
//---------------------------------------------------------------------------------------------------------------------------------
void CombDlg::COMB_InitList()
{
	layer_Index.clear();
	layer_titles.clear();
	layer_items.clear();

	m_wndListView.DeleteAllItems();
	m_imgList.Destroy();
	m_imgList.Create(36, 36, ILC_COLOR32, 2, 2);

	HDC hdc = g_combDlg->GetDC();

	int itemNum = 0;

	ComPtr<IImage> image = g_frameWork->imageEditor.m_images[g_frameWork->imageEditor.m_currentIndex].Image;
	for (int i = (int)image->GetOperationsSize()-1; i >=0 ; i--)
	{
		ComPtr<IImageOperation> operation = image->Get_ith_Operation(i);
		ComPtr<IImageLayerOperation> layerOperation;
		ComPtr<IImageTextOperation> textOperation;
		ComPtr<IDrawGeometryOperation> geoOperation;
		ComPtr<IImagePicsOperation> picsOperation;
		ComPtr<IImageSVGOperation> svgOperation;

		HBITMAP hbitmap;

		//---------------------------------------------------------------------------------------------------------------------------------
		// Pic layer
		//---------------------------------------------------------------------------------------------------------------------------------
		if (SUCCEEDED(operation.QueryInterface(&layerOperation)) && 
			!SUCCEEDED(operation.QueryInterface(&picsOperation)) && 
			!SUCCEEDED(operation.QueryInterface(&svgOperation)))
		{
			CBLayerPtr bp;
			layerOperation->GetLayerEquipment(&bp);
			CString showName;

			if (bp.m_layerIfPasting == true)
			{
				showName = L"图片层：From 剪贴板";

				ComPtr<IWICBitmapFlipRotator> wicBitmapFRer;
				ComPtr<IWICBitmapScaler > scaler;
				ComPtr<IWICBitmapSource> source = bp.m_layerWicBitmap;
				g_pWICFactory->CreateBitmapFlipRotator(&wicBitmapFRer);
				g_pWICFactory->CreateBitmapScaler(&scaler);
				wicBitmapFRer->Initialize(source, WICBitmapTransformFlipVertical);
				scaler->Initialize(wicBitmapFRer, 36, 36, WICBitmapInterpolationModeCubic);
				hbitmap = Direct2DUtility::CreateHBITMAP(scaler);
			}
			else
			{
				CString name = bp.m_layerPicFileName;
				CString pname = name.Right(name.GetLength() - name.ReverseFind(L'\\') - 1);
				showName = L"图片层: " + pname;
				hbitmap = COMB_MakeHBITMAP(bp.m_layerPicFileName.GetBuffer(200));
			}

			WCHAR* buf = showName.GetBuffer(COMBDLG_BUFLEN);
			int stop = GetStopCharLoc(buf, SHOWPIXELS, hdc);
			buf[stop] = L'\0';
			wcscpy_s(namebuf, COMBDLG_BUFLEN, buf);

			layer_Index.push_back(i);
			layer_items.push_back(operation);
			layer_titles.push_back(CString(namebuf));

			COMB_InsertListView(hbitmap, namebuf, itemNum);

			itemNum++;
		}

		//---------------------------------------------------------------------------------------------------------------------------------
		// Pics box layer
		//---------------------------------------------------------------------------------------------------------------------------------
		layerOperation = nullptr;
		picsOperation = nullptr;
		if (SUCCEEDED(operation.QueryInterface(&layerOperation)) && SUCCEEDED(operation.QueryInterface(&picsOperation)))
		{
			CBLayerPtr bp;
			layerOperation->GetLayerEquipment(&bp);
			CString showName;

			CString name = bp.m_layerPicFileName;
			CString pname = name.Right(name.GetLength() - name.ReverseFind(L'\\') - 1);
			showName = L"图片盒: " + pname;

			WCHAR* buf = showName.GetBuffer(COMBDLG_BUFLEN);
			int stop = GetStopCharLoc(buf, SHOWPIXELS, hdc);
			buf[stop] = L'\0';
			wcscpy_s(namebuf, COMBDLG_BUFLEN, buf);

			layer_Index.push_back(i);
			layer_titles.push_back(CString(namebuf));
			layer_items.push_back(operation);

			hbitmap = COMB_MakeHBITMAP(bp.m_layerPicFileName.GetBuffer(200));
			COMB_InsertListView(hbitmap, namebuf, itemNum);

			itemNum++;
		}

		//---------------------------------------------------------------------------------------------------------------------------------
		// SVG layer
		//---------------------------------------------------------------------------------------------------------------------------------
		layerOperation = nullptr;
		svgOperation = nullptr;
		if (SUCCEEDED(operation.QueryInterface(&layerOperation)) && 
			SUCCEEDED(operation.QueryInterface(&svgOperation)))
		{
			CBLayerPtr bp;
			layerOperation->GetLayerEquipment(&bp);
			CString showName;

			CString name = bp.m_layerPicFileName;
			CString pname = name.Right(name.GetLength() - name.ReverseFind(L'\\') - 1);
			showName = L"SVG 图片: " + pname;

			WCHAR* buf = showName.GetBuffer(COMBDLG_BUFLEN);
			int stop = GetStopCharLoc(buf, SHOWPIXELS, hdc);
			buf[stop] = L'\0';
			wcscpy_s(namebuf, COMBDLG_BUFLEN, buf);

			layer_Index.push_back(i);
			layer_titles.push_back(CString(namebuf));
			layer_items.push_back(operation);

			hbitmap = COMB_MakeHBITMAP(L"K:\\娱乐\\vs2022\\wtleditor\\wtleditor\\res\\t.png");
			COMB_InsertListView(hbitmap, namebuf, itemNum);

			itemNum++;
		}

		//---------------------------------------------------------------------------------------------------------------------------------
		// Text layer
		//---------------------------------------------------------------------------------------------------------------------------------
		if (SUCCEEDED(operation.QueryInterface(&textOperation)))
		{
			CBTextPtr bp;
			textOperation->GetTextEquipment(&bp);
			CString name = bp.m_textString;
			name.Replace(L"\r", L"");
			name.Replace(L"\n", L"");
			CString showName = L"文字层: " + name;

			WCHAR* buf = showName.GetBuffer(COMBDLG_BUFLEN);
			int stop = GetStopCharLoc(buf, SHOWPIXELS, hdc);
			buf[stop] = L'\0';
			wcscpy_s(namebuf, COMBDLG_BUFLEN, buf);

			layer_Index.push_back(i);
			layer_titles.push_back(CString(namebuf));
			layer_items.push_back(operation);

			hbitmap = COMB_MakeHBITMAP(L"K:\\娱乐\\vs2022\\wtleditor\\wtleditor\\res\\text.png");
			COMB_InsertListView(hbitmap, namebuf, itemNum);

			itemNum++;
		}

		//---------------------------------------------------------------------------------------------------------------------------------
		// Geo layer
		//---------------------------------------------------------------------------------------------------------------------------------
		if (SUCCEEDED(operation.QueryInterface(&geoOperation)))
		{
			WCHAR* namebuf = L"画笔层";
			layer_Index.push_back(i);
			layer_titles.push_back(CString(namebuf));
			layer_items.push_back(operation);

			hbitmap = COMB_MakeHBITMAP(L"K:\\娱乐\\vs2022\\wtleditor\\wtleditor\\res\\pen-2.png");
			COMB_InsertListView(hbitmap, namebuf, itemNum);
			itemNum++;
		}
	}

	layer_Index.push_back(-1);
	layer_titles.push_back(CString(L""));
	layer_items.push_back(nullptr);

	HBITMAP hbitmap = COMB_MakeHBITMAP(L"K:\\娱乐\\vs2022\\wtleditor\\wtleditor\\res\\o.png");
	COMB_InsertListView(hbitmap, L"", itemNum);
	itemNum++;

	m_wndListView.SetImageList(m_imgList.m_hImageList, LVSIL_SMALL);

	//m_layers.DeclareItemsNumber(itemNum);
	//for (int i = 0; i < itemNum; i++)
	//{
	//	m_layers.SetItemText(i, layer_titles[i], true);
	//}

	//---------------------------------------------------------------------------------------------------------------------------------
	// 把ComboBox的显示设置为最末一项的Blank item上。
	//---------------------------------------------------------------------------------------------------------------------------------
	//m_layers.Select(ZEROPLACE, true);
	m_wndListView.SelectItem(ZEROPLACE);

	g_frameWork->COMB_SetSpinnerOpacity(0);
	g_frameWork->COMB_SetSpinnerDuration(0);
}

void	CombDlg::COMB_UpdateTextLayerString(ComPtr<IImageOperation> item)
{
	for (int i = 0; i < ZEROPLACE; i++)
	{
		if (layer_items[i] == item)
		{
			ComPtr<IImageTextOperation> textOperation;
			if (SUCCEEDED(item.QueryInterface(&textOperation)))
			{
				HDC hdc = g_combDlg->GetDC();

				CBTextPtr bp;
				textOperation->GetTextEquipment(&bp);

				CString name = bp.m_textString;
				name.Replace(L"\r", L"");
				name.Replace(L"\n", L"");
				CString showName = L"文字层: " + name;

				WCHAR* buf = showName.GetBuffer(COMBDLG_BUFLEN);
				int stop = GetStopCharLoc(buf, SHOWPIXELS, hdc);
				buf[stop] = L'\0';
				wcscpy_s(namebuf, COMBDLG_BUFLEN, buf);

				LVITEM lvi = { 0 };
				lvi.mask = LVIF_TEXT;
				lvi.iItem = i;
				lvi.iSubItem = 0;
				lvi.pszText = namebuf;
				lvi.cchTextMax = MAX_PATH;
				m_wndListView.SetItem(&lvi);
			}
		}
	}
}

void CombDlg::COMB_InsertListView(HBITMAP hbitmap, WCHAR* namebuf, int itemNum)
{
	m_imgList.Add(hbitmap);
	int n;
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	lvi.iItem = itemNum;
	lvi.iSubItem = 0;
	lvi.iImage = itemNum;
	lvi.pszText = namebuf;
	lvi.cchTextMax = MAX_PATH;
	n = m_wndListView.InsertItem(&lvi);
	//m_wndListView.AddItem(n, 1, namebuf);
}

//---------------------------------------------------------------------------------------------------------------------------------
// 程序开始时初始化 BomboBox。
//---------------------------------------------------------------------------------------------------------------------------------
void CombDlg::COMB_EmptyList()
{
	layer_Index.clear();
	layer_titles.clear();
	layer_items.clear();

	m_wndListView.DeleteAllItems();
	m_imgList.Destroy();
	m_imgList.Create(36, 36, ILC_COLOR32, 2, 2);

	layer_Index.push_back(-1);
	layer_titles.push_back(CString(L""));
	layer_items.push_back(nullptr);

	HBITMAP hbitmap = COMB_MakeHBITMAP(L"K:\\娱乐\\vs2022\\wtleditor\\wtleditor\\res\\o.png");
	COMB_InsertListView(hbitmap, L"", 0);
	m_wndListView.SelectItem(0);

	m_wndListView.SetImageList(m_imgList.m_hImageList, LVSIL_SMALL);

	//m_layers.DeclareItemsNumber(1);
	//m_layers.SetItemText(0, layer_titles[0], true);
	//m_layers.Select(0, true);

	COMB_EmptySpinners();
}

void CombDlg::COMB_EmptySpinners()
{
	g_frameWork->COMB_SetSpinnerOpacity(0);
	g_frameWork->COMB_SetSpinnerDuration(0);
}

//---------------------------------------------------------------------------------------------------------------------------------
// 设置 item 的Spinner数值显示。
//---------------------------------------------------------------------------------------------------------------------------------
void CombDlg::COMB_SetSpinners(ComPtr<IImageOperation> item)
{
	if (item == nullptr)
		return;

	g_frameWork->COMB_SetSpinnerOpacity((LONG)(100 * item->GetOpacity()));
	g_frameWork->COMB_SetSpinnerDuration((LONG)(item->GetDuration()));
}

//---------------------------------------------------------------------------------------------------------------------------------
// 激活动作包括：设置ComboBox，设置Spinners值，设置 active 框。
//---------------------------------------------------------------------------------------------------------------------------------
extern bool g_bCombDialogShow;

void CombDlg::COMB_ActivateItem(ComPtr<IImageOperation> item)
{
	if (item == nullptr)
	{
		COMB_DeactivateEveryItem();
		g_frameWork->imageEditor.m_currentOperation = nullptr;
		return;
	}

	for (int i = 0; i < ZEROPLACE; i++)
	{
		if (item == layer_items.at(i))
		{
			ComPtr<IImageSVGOperation> svgOpera;
			if (SUCCEEDED(item->QueryInterface(&svgOpera)))
			{
				combDlg_ptr->trackBar_.ShowWindow(SW_SHOW);
				combDlg_ptr->upDownQuality3_.ShowWindow(SW_SHOW);
				combDlg_ptr->edit3_.ShowWindow(SW_SHOW);
				int d3 = svgOpera->GetSVGOffSet();
				combDlg_ptr->trackBar_.setValue(d3);
				combDlg_ptr->upDownQuality3_.SetPos(d3);
				combDlg_ptr->setQualityEditValue3(d3);
			}
			else
			{
				combDlg_ptr->trackBar_.ShowWindow(SW_HIDE);
				combDlg_ptr->upDownQuality3_.ShowWindow(SW_HIDE);
				combDlg_ptr->edit3_.ShowWindow(SW_HIDE);
			}

			COMB_SetSpinners(item);
			g_frameWork->imageEditor.PAST_SelectLayer(item);
			m_wndListView.SelectItem(i);
			return;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
// 激活（设置 ComboBox，设置 Spinners，设置 Active 框）除 item 以外的另一个可显示 Operation（Text, Pic, 以及 Geo），
// 如果其它已经没有 operation 了，则把 m_currentOperation 设置为 nullptr。
//---------------------------------------------------------------------------------------------------------------------------------
void CombDlg::COMB_DeactivateItemWithFollower(ComPtr<IImageOperation> item)
{
	if (item == nullptr)
	{
		COMB_DeactivateEveryItem();
		g_frameWork->imageEditor.m_currentOperation = nullptr;
		return;
	}

	COMB_DeactivateEveryItem();

	for (int i = 0; i < ZEROPLACE; i++)
	{
		if (item != layer_items.at(i))
		{
			//m_layers.Select(i, true);
			m_wndListView.SelectItem(i);

			COMB_SetSpinners(layer_items.at(i));
			g_frameWork->imageEditor.PAST_SelectLayer(layer_Index.at(i));
			return;
		}
	}

	g_frameWork->imageEditor.m_currentOperation = nullptr;
}

void CombDlg::COMB_DeactivateEveryItem()
{
	//m_layers.Select(ZEROPLACE, true);
	m_wndListView.SelectItem(ZEROPLACE);

	COMB_EmptySpinners();
	g_frameWork->imageEditor.PAST_UnselectLayer();
}

void CombDlg::COMB_SetItemActive(ComPtr<IImageOperation> item, bool active)
{
	if (active == true)
	{
		COMB_ActivateItem(item);
	}
	else
	{
		COMB_DeactivateEveryItem();
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
// ComboBox 选择了 item之后的动作。
// 1）显示 Spinners 值数值。
// 2）显示激活的 item 的 active 框。
// 3）相应激活 Ribbon 里的各个按钮。
//---------------------------------------------------------------------------------------------------------------------------------
void CombDlg::OnPicLayerSelected(UINT layerId)
{
	int d3 = 0;
	combDlg_ptr->trackBar_.setValue(d3);
	combDlg_ptr->upDownQuality3_.SetPos(d3);
	combDlg_ptr->setQualityEditValue3(d3);

	if (g_frameWork->imageEditor.GetOperationType() == ImageOperationTypeText ||
		g_frameWork->imageEditor.GetOperationType() == ImageOperationTypeLayer)
	{
		COMB_ActivateItem(layer_items.at(min(ZEROPLACE, (int)layerId)));
		g_frameWork->imageEditor.UpdateUIFramework();
	}

	ComPtr<IImageOperation> opera = layer_items.at(min(ZEROPLACE, (int)layerId));
	if (opera == nullptr)
	{
		return;
	}

	ComPtr<IImageSVGOperation> svgOpera;
	if (SUCCEEDED(opera->QueryInterface(&svgOpera)))
	{
		int d3 = svgOpera->GetSVGOffSet();
		combDlg_ptr->trackBar_.setValue(d3);
		combDlg_ptr->upDownQuality3_.SetPos(d3);
		combDlg_ptr->setQualityEditValue3(d3);
	}
}

LRESULT CombDlg::OnLVItemClick(int, LPNMHDR pnmh, BOOL&)
{
	if (pnmh->hwndFrom != m_wndListView.m_hWnd)
		return 0;

	POINT pt;
	::GetCursorPos((LPPOINT)&pt);
	m_wndListView.ScreenToClient(&pt);

	LVHITTESTINFO lvhti;
	lvhti.pt = pt;
	m_wndListView.HitTest(&lvhti);

	if (lvhti.flags & LVHT_ONITEM)
	{
		OnPicLayerSelected(lvhti.iItem);
		m_wndListView.SelectItem(lvhti.iItem);
	}
	return 0;
}

static LRESULT CALLBACK NewQualityEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_GETDLGCODE)
	{
		return (DLGC_WANTALLKEYS | CallWindowProc(OldWndProc, hWnd, message, wParam, lParam));
	}

	if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
	{
		if (wParam == VK_RETURN)
		{
			if (hWnd == combDlg_ptr->edit3_.m_hWnd)
			{
				int d3 = combDlg_ptr->getQualityValue3();
				if (d3 > 100) d3 = 100;
				if (d3 < -500) d3 = -500;
				combDlg_ptr->upDownQuality3_.SetPos(d3);
				combDlg_ptr->setQualityEditValue3(d3);
				combDlg_ptr->trackBar_.setValue(d3);
			}
			return TRUE;
		}

		if ((wParam < '0' || wParam > '9') && wParam != '-' && wParam != VK_LEFT && wParam != VK_RIGHT &&
			wParam != VK_DELETE && wParam != VK_BACK)
			return TRUE;
	}

	return CallWindowProc(OldWndProc, hWnd, message, wParam, lParam);
}

LRESULT CombDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	//-------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------
	CRect rec;

	GetDlgRect(IDC_STATIC_33, &rec);
	edit3_.Create(m_hWnd, CRect(CPoint(rec.left, rec.top), CSize(60, 20)), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		0, ContentEditupDown3);

	//UDS_SETBUDDYINT: 自动设置Edit的整型值
	upDownQuality3_.Create(m_hWnd, CRect(CPoint(), CSize()), NULL, WS_CHILDWINDOW | WS_BORDER | WS_VISIBLE
		| UDS_AUTOBUDDY | UDS_ALIGNRIGHT
		| UDS_ARROWKEYS | UDS_HOTTRACK, 0, ContentQualityUpDown3);
	upDownQuality3_.SetRange(-500, 100);

	upDownQuality3_.SetPos(QUALITY_INITIAL_VALUE);
	setQualityEditValue3(QUALITY_INITIAL_VALUE);

	combDlg_ptr = this;

	OldWndProc = (WNDPROC)edit3_.SetWindowLongPtr(GWLP_WNDPROC, (LONG_PTR)NewQualityEditProc);

	GetDlgRect(IDC_STATIC_34, &rec);
	trackBar_.Create(m_hWnd, rec, NULL, WS_CHILD | WS_VISIBLE);
	function<void(int)> func(bind(&CombDlg::onTrackBarValueChange, this, placeholders::_1));
	trackBar_.setFuncValueChanging(func);
	trackBar_.updateLayout(rec);
	trackBar_.setValueRange(-500, 100);
	trackBar_.setValue(QUALITY_INITIAL_VALUE);
	trackBar_.ShowWindow(SW_SHOW);

	//-------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------
	RECT recClient;
	GetClientRect(&recClient);

	int left = rec.left;
	CRect rect = { left, 60, left + 32, 60 + 32 };
	initialPost_ = new CPostView(m_hWnd, &rect, L"K:\\娱乐\\vs2022\\wtleditor\\wtleditor\\res\\layers.png");

	RECT listViewRect = { left, 105, left + 300, 105 + 200 };
	m_wndListView.Create(m_hWnd, listViewRect, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		LVS_REPORT | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS,
		WS_EX_CLIENTEDGE);
	//m_wndListView.SetExtendedListViewStyle(LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE); // | LVS_EX_UNDERLINEHOT);

	m_wndListView.InsertColumn(0, L"pic", LVCFMT_LEFT, 200, 0);
	//m_wndListView.InsertColumn(1, L"title", LVCFMT_LEFT, 160, 1);

	m_imgList.Create(36, 36, ILC_COLOR32, 2, 2);

	return TRUE;
}

void CombDlg::onTrackBarValueChange(int val)
{
	upDownQuality3_.SetPos(val);
	setQualityEditValue3(val);

	ComPtr<IImageSVGOperation> svgOpera;
	if (SUCCEEDED(g_frameWork->imageEditor.m_currentOperation->QueryInterface(&svgOpera)))
	{
		svgOpera->SetSVGOffSet(val);
	}
}
