#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

namespace {

	struct IconModeOption {
		const char* shortLabel;
		IconType type;
	};

	// Index in this array must match IconType's numeric value (Cube=0 .. Swing=7).
	constexpr IconModeOption ICON_MODES[] = {
		{"Cubo", IconType::Cube},
		{"Nave", IconType::Ship},
		{"Bola", IconType::Ball},
		{"Ovni", IconType::Ufo},
		{"Onda", IconType::Wave},
		{"Robot", IconType::Robot},
		{"Arana", IconType::Spider},
		{"Swing", IconType::Swing},
	};
	constexpr int ICON_MODE_COUNT = sizeof(ICON_MODES) / sizeof(ICON_MODES[0]);

	// Kept conservative: some icon categories (eg. Swing) historically have far
	// fewer unlocked frames than Cube, and requesting a nonexistent sprite frame
	// can leave the sprite blank. This range is safely below the smallest category.
	constexpr int MIN_ICON_ID = 1;
	constexpr int MAX_ICON_ID = 20;

	int wrapIconId(int id, int delta) {
		int const span = MAX_ICON_ID - MIN_ICON_ID + 1;
		int idx = (id - MIN_ICON_ID + delta) % span;
		if (idx < 0) idx += span;
		return MIN_ICON_ID + idx;
	}

	void applyIconFrame(PlayerObject* player, int modeIndex, int iconId) {
		if (!player) return;
		switch (modeIndex) {
			case 0: player->updatePlayerFrame(iconId); break;
			case 1: player->updatePlayerShipFrame(iconId); break;
			case 2: player->updatePlayerRollFrame(iconId); break;
			case 3: player->updatePlayerBirdFrame(iconId); break;
			case 4: player->updatePlayerDartFrame(iconId); break;
			case 5: player->updatePlayerRobotFrame(iconId); break;
			case 6: player->updatePlayerSpiderFrame(iconId); break;
			case 7: player->updatePlayerSwingFrame(iconId); break;
		}
	}

	void applyIconIdToGameManager(int modeIndex, int iconId) {
		auto gm = GameManager::sharedState();
		switch (modeIndex) {
			case 0: gm->setPlayerFrame(iconId); break;
			case 1: gm->setPlayerShip(iconId); break;
			case 2: gm->setPlayerBall(iconId); break;
			case 3: gm->setPlayerBird(iconId); break;
			case 4: gm->setPlayerDart(iconId); break;
			case 5: gm->setPlayerRobot(iconId); break;
			case 6: gm->setPlayerSpider(iconId); break;
			case 7: gm->setPlayerSwing(iconId); break;
		}
	}

	int currentIconIdForMode(int modeIndex) {
		auto gm = GameManager::sharedState();
		int id = 1;
		switch (modeIndex) {
			case 0: id = gm->getPlayerFrame(); break;
			case 1: id = gm->getPlayerShip(); break;
			case 2: id = gm->getPlayerBall(); break;
			case 3: id = gm->getPlayerBird(); break;
			case 4: id = gm->getPlayerDart(); break;
			case 5: id = gm->getPlayerRobot(); break;
			case 6: id = gm->getPlayerSpider(); break;
			case 7: id = gm->getPlayerSwing(); break;
		}
		if (id < MIN_ICON_ID || id > MAX_ICON_ID) id = MIN_ICON_ID;
		return id;
	}

	void forEachLivePlayer(std::function<void(PlayerObject*)> const& fn) {
		if (auto pl = PlayLayer::get()) {
			if (pl->m_player1) fn(pl->m_player1);
			if (pl->m_player2) fn(pl->m_player2);
		}
	}

}

class IconSwitcherPopup : public geode::Popup {
protected:
	int m_modeIndex = 0;
	int m_iconId = 1;
	ccColor3B m_color1 = {255, 255, 255};
	ccColor3B m_color2 = {0, 255, 255};
	bool m_glow = false;

	SimplePlayer* m_preview = nullptr;
	CCLabelBMFont* m_idLabel = nullptr;
	CCMenu* m_modeMenu = nullptr;
	ButtonSprite* m_glowSprite = nullptr;

	bool init() {
		if (!Popup::init(360.f, 260.f)) {
			return false;
		}

		this->setTitle("Cambiar Icono");

		auto gm = GameManager::sharedState();
		m_color1 = gm->colorForIdx(gm->getPlayerColor());
		m_color2 = gm->colorForIdx(gm->getPlayerColor2());
		m_glow = gm->getPlayerGlow();
		m_iconId = currentIconIdForMode(m_modeIndex);

		m_preview = SimplePlayer::create(m_iconId);
		m_preview->setPosition({m_size.width / 2, m_size.height - 70.f});
		m_preview->setScale(1.4f);
		m_mainLayer->addChild(m_preview);

		m_modeMenu = CCMenu::create();
		m_modeMenu->setLayout(
			RowLayout::create()
				->setGap(3.f)
				->setAxisAlignment(AxisAlignment::Center)
				->setCrossAxisOverflow(false)
		);
		for (int i = 0; i < ICON_MODE_COUNT; i++) {
			auto btnSpr = ButtonSprite::create(ICON_MODES[i].shortLabel, 0, false, "bigFont.fnt", "GJ_button_01.png", 0.f, 0.6f);
			auto btn = CCMenuItemSpriteExtra::create(
				btnSpr, this, menu_selector(IconSwitcherPopup::onSelectMode)
			);
			btn->setTag(i);
			m_modeMenu->addChild(btn);
		}
		m_modeMenu->setContentSize({m_size.width - 20.f, 30.f});
		m_modeMenu->setLayout(
			RowLayout::create()
				->setGap(3.f)
				->setAxisAlignment(AxisAlignment::Center)
				->setCrossAxisOverflow(false)
		);
		m_modeMenu->setPosition({m_size.width / 2, m_size.height - 108.f});
		m_mainLayer->addChild(m_modeMenu);

		auto idMenu = CCMenu::create();
		auto prevBtn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("<"), this, menu_selector(IconSwitcherPopup::onIconIdPrev)
		);
		auto nextBtn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create(">"), this, menu_selector(IconSwitcherPopup::onIconIdNext)
		);
		m_idLabel = CCLabelBMFont::create(this->idLabelText().c_str(), "bigFont.fnt");
		m_idLabel->setScale(0.5f);
		idMenu->addChild(prevBtn);
		idMenu->addChild(m_idLabel);
		idMenu->addChild(nextBtn);
		idMenu->setContentSize({160.f, 30.f});
		idMenu->setLayout(
			RowLayout::create()->setGap(10.f)->setAxisAlignment(AxisAlignment::Center)
		);
		idMenu->setPosition({m_size.width / 2, m_size.height - 150.f});
		m_mainLayer->addChild(idMenu);

		auto colorMenu = CCMenu::create();
		auto color1Btn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("Color 1"), this, menu_selector(IconSwitcherPopup::onPickColor1)
		);
		auto color2Btn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("Color 2"), this, menu_selector(IconSwitcherPopup::onPickColor2)
		);
		colorMenu->addChild(color1Btn);
		colorMenu->addChild(color2Btn);
		colorMenu->setContentSize({m_size.width - 40.f, 30.f});
		colorMenu->setLayout(
			RowLayout::create()->setGap(10.f)->setAxisAlignment(AxisAlignment::Center)
		);
		colorMenu->setPosition({m_size.width / 2, m_size.height - 190.f});
		m_mainLayer->addChild(colorMenu);

		auto glowMenu = CCMenu::create();
		m_glowSprite = ButtonSprite::create(this->glowLabelText().c_str());
		auto glowBtn = CCMenuItemSpriteExtra::create(
			m_glowSprite, this, menu_selector(IconSwitcherPopup::onToggleGlow)
		);
		glowMenu->addChild(glowBtn);
		glowMenu->setContentSize({160.f, 30.f});
		glowMenu->setLayout(
			RowLayout::create()->setAxisAlignment(AxisAlignment::Center)
		);
		glowMenu->setPosition({m_size.width / 2, m_size.height - 225.f});
		m_mainLayer->addChild(glowMenu);

		this->refreshPreview();

		return true;
	}

	std::string idLabelText() {
		return "ID: " + std::to_string(m_iconId);
	}

	std::string glowLabelText() {
		return m_glow ? "Glow: ON" : "Glow: OFF";
	}

	void refreshPreview() {
		if (!m_preview) return;
		m_preview->updatePlayerFrame(m_iconId, ICON_MODES[m_modeIndex].type);
		m_preview->setColors(m_color1, m_color2);
		if (m_idLabel) m_idLabel->setString(this->idLabelText().c_str());
	}

	void applyLive() {
		int const modeIndex = m_modeIndex;
		int const iconId = m_iconId;
		ccColor3B const color1 = m_color1;
		ccColor3B const color2 = m_color2;
		bool const glow = m_glow;

		auto gm = GameManager::sharedState();
		gm->setPlayerGlow(glow);

		forEachLivePlayer([=](PlayerObject* player) {
			applyIconFrame(player, modeIndex, iconId);
			player->setColor(color1);
			player->setSecondColor(color2);
			player->updatePlayerGlow();
			player->updateGlowColor();
		});
	}

	void onSelectMode(CCObject* sender) {
		m_modeIndex = sender->getTag();
		m_iconId = currentIconIdForMode(m_modeIndex);
		this->refreshPreview();
		this->applyLive();
	}

	void onIconIdPrev(CCObject*) {
		m_iconId = wrapIconId(m_iconId, -1);
		applyIconIdToGameManager(m_modeIndex, m_iconId);
		this->refreshPreview();
		this->applyLive();
	}

	void onIconIdNext(CCObject*) {
		m_iconId = wrapIconId(m_iconId, 1);
		applyIconIdToGameManager(m_modeIndex, m_iconId);
		this->refreshPreview();
		this->applyLive();
	}

	void onPickColor1(CCObject*) {
		auto picker = ColorPickPopup::create(m_color1);
		picker->setCallback([this](ccColor4B const& color) {
			m_color1 = {color.r, color.g, color.b};
			this->refreshPreview();
			this->applyLive();
		});
		picker->show();
	}

	void onPickColor2(CCObject*) {
		auto picker = ColorPickPopup::create(m_color2);
		picker->setCallback([this](ccColor4B const& color) {
			m_color2 = {color.r, color.g, color.b};
			this->refreshPreview();
			this->applyLive();
		});
		picker->show();
	}

	void onToggleGlow(CCObject*) {
		m_glow = !m_glow;
		if (m_glowSprite) m_glowSprite->setString(this->glowLabelText().c_str());
		this->applyLive();
	}

public:
	static IconSwitcherPopup* create() {
		auto ret = new IconSwitcherPopup();
		if (ret->init()) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}
};

class $modify(IconSwitcherPauseLayer, PauseLayer) {
	bool init(bool unfocused) {
		if (!PauseLayer::init(unfocused)) {
			return false;
		}

		auto menu = this->getChildByID("left-button-menu");
		if (!menu) {
			return true;
		}

		auto btn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("Icono", 0, false, "bigFont.fnt", "GJ_button_04.png", 0.f, 0.8f),
			this,
			menu_selector(IconSwitcherPauseLayer::onIconSwitcher)
		);
		btn->setID("icon-switcher-button"_spr);
		menu->addChild(btn);
		menu->updateLayout();

		return true;
	}

	void onIconSwitcher(CCObject*) {
		auto popup = IconSwitcherPopup::create();
		popup->show();
	}
};
