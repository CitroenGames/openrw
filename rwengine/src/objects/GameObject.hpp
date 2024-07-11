#ifndef _RWENGINE_GAMEOBJECT_HPP_
#define _RWENGINE_GAMEOBJECT_HPP_

#include <limits>
#include <variant>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include <rw/debug.hpp>
#include <rw/forward.hpp>

#include <data/ModelData.hpp>
#include <engine/Animator.hpp>
#include <objects/ObjectTypes.hpp>

class GameWorld;

/**
 * @brief Base data and interface for all world "objects" like vehicles, peds.
 *
 * Contains handle to the world, and other useful properties like water level
 * tracking used to make tunnels work.
 */
class GameObject {
    GameObjectID objectID = 0;

    BaseModelInfo* modelinfo_;

    using Model = std::variant<AtomicPtr, ClumpPtr>;
    Model model_;

    static const AtomicPtr NullAtomic;
    static const ClumpPtr NullClump;

protected:
    void changeModelInfo(BaseModelInfo* next) {
        modelinfo_ = next;
    }

public:
    glm::vec3 position;
    glm::quat rotation;

    GameWorld* engine = nullptr;

    std::unique_ptr<Animator> animator;  /// Object's animator.

    bool inWater = false;

    /**
     * @brief stores the height of water at the last tick
     */
    float _lastHeight = std::numeric_limits<float>::max();

    /**
     * Should object be rendered?
     */
    bool visible = true;

    GameObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot,
               BaseModelInfo* modelinfo)
        : modelinfo_(modelinfo), position(pos), rotation(rot), engine(engine) {
        if (modelinfo_) {
            modelinfo_->addReference();
        }
    }

    virtual ~GameObject();

    GameObjectID getGameObjectID() const {
        return objectID;
    }
    /**
     * Do not call this, use GameWorld::insertObject
     */
    void setGameObjectID(GameObjectID id) {
        objectID = id;
    }

    int getScriptObjectID() const {
        return getGameObjectID();
    }

    template <class T>
    T* getModelInfo() const {
        return static_cast<T*>(modelinfo_);
    }

    const Model& getModel() const {
        return model_;
    }

     const AtomicPtr& getAtomic() const {
        if (auto atomic = std::get_if<AtomicPtr>(&model_))
        {
            return *atomic;
        }
        return NullAtomic;
    }

    const ClumpPtr& getClump() const {
        if (auto clump = std::get_if<ClumpPtr>(&model_))
        {
            return *clump;
        }
        return NullClump;
    }

    void setModel(const AtomicPtr& model) {
        model_ = model;
    }

    void setModel(const ClumpPtr& model) {
        model_ = model;
    }

    /**
     * @brief Enumeration of possible object types.
     */
    enum Type {
        Instance,
        Character,
        Vehicle,
        Pickup,
        Projectile,
        Cutscene,
        Unknown
    };

    /**
     * @brief determines what type of object this is.
     * @return one of Type
     */
    virtual Type type() const {
        return Unknown;
    }

    virtual void setPosition(const glm::vec3& pos);

    const glm::vec3& getPosition() const {
        return position;
    }

    const glm::quat& getRotation() const {
        return rotation;
    }
    virtual void setRotation(const glm::quat& orientation);

    float getHeading() const;
    /**
     * @brief setHeading Rotates the object to face heading, in degrees.
     */
    void setHeading(float heading);

    /**
     * @brief getCenterOffset Returns the offset from center of mass to base of model
     * This function should be overwritten by a derived class
     */
    virtual glm::vec3 getCenterOffset() {
        return glm::vec3(0.f, 0.f, 1.f);
    }

    /**
     * @brief applyOffset Applies the offset from getCenterOffset to the object
     */
    void applyOffset() {
         setPosition(getPosition() + getCenterOffset());
    }

    struct DamageInfo {
        enum class DamageType {
            Explosion, Burning, Bullet, Physics, Melee
        };

        /**
         * World position of damage
         */
        glm::vec3 damageLocation{};

        /**
         * World position of the source (used for direction)
         */
        glm::vec3 damageSource{};

        /**
         * Magnitude of destruction
         */
        float hitpoints;

        /**
         * Type of the damage
         */
        DamageType type;

        /**
         * Physics impulse.
         */
        float impulse;

        DamageInfo(DamageType type, const glm::vec3 &location,
                   const glm::vec3 &source, float damage, float impulse = 0.f)
            : damageLocation(location), damageSource(source), hitpoints(damage),
              type(type), impulse(impulse) {}
    };

    virtual bool takeDamage(const DamageInfo& damage) {
        RW_UNUSED(damage);
        return false;
    }

    virtual bool isAnimationFixed() const {
        return true;
    }

    virtual bool isInWater() const {
        return inWater;
    }

    virtual void tick(float dt) = 0;

    enum ObjectLifetime {
        /// lifetime has not been set
        UnknownLifetime,
        /// Generic background pedestrians
        TrafficLifetime,
        /// Part of a mission
        MissionLifetime,
        /// Is owned by the player (or is the player)
        PlayerLifetime
    };

    void setLifetime(ObjectLifetime ol) {
        lifetime = ol;
    }

    ObjectLifetime getLifetime() const {
        return lifetime;
    }

    /// Returns true if the object is not referenced by a script or player
    virtual bool canBeRemoved() const {
        switch (lifetime) {
            case MissionLifetime:
            case PlayerLifetime:
                return false;
            default:
                return true;
        }
    }

    void updateTransform(const glm::vec3& pos, const glm::quat& rot);

private:
    ObjectLifetime lifetime = GameObject::UnknownLifetime;
};

#endif  // __GAMEOBJECTS_HPP__
