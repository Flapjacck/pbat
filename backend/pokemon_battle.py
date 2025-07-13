import random
import math
import time
from typing import Dict, List, Tuple, Optional
import requests
import json

class Pokemon:
    def __init__(self, name: str, pokemon_data: dict, ivs: dict, evs: dict, moves: list, level: int = 50, held_item: str = None):
        self.name = name.capitalize()
        self.pokemon_data = pokemon_data
        self.ivs = ivs
        self.evs = evs
        self.moves = moves
        self.level = level
        self.types = [t['type']['name'] for t in pokemon_data['types']]
        self.held_item = held_item
        
        # Calculate actual stats
        self.stats = self._calculate_stats()
        self.max_hp = self.stats['hp']
        self.current_hp = self.max_hp
        
        # Status conditions
        self.status = None  # poison, burn, paralysis, sleep, freeze, confusion, flinch
        self.status_turns = 0
        self.confusion_turns = 0
        self.flinch = False
        
        # Battle stats modifications (temporary)
        self.stat_modifiers = {
            'attack': 0,
            'defense': 0,
            'special-attack': 0,
            'special-defense': 0,
            'speed': 0,
            'accuracy': 0,
            'evasion': 0
        }
        
        # Critical hit ratio (base 1/24, stage 0)
        self.crit_stage = 0
        
        # Move data cache
        self.move_data = {}
        self._load_move_data()
        
        # Held item data
        self.item_data = self._load_item_data() if held_item else {}
    
    def _calculate_stats(self) -> dict:
        """Calculate actual stats using Pokemon formula"""
        calculated_stats = {}
        
        for stat_info in self.pokemon_data['stats']:
            stat_name = stat_info['stat']['name']
            base_stat = stat_info['base_stat']
            iv = self.ivs.get(stat_name, 0)
            ev = self.evs.get(stat_name, 0)
            
            if stat_name == 'hp':
                # HP formula
                calculated_stats[stat_name] = math.floor(
                    ((2 * base_stat + iv + math.floor(ev / 4)) * self.level) / 100
                ) + self.level + 10
            else:
                # Other stats formula
                calculated_stats[stat_name] = math.floor(
                    (((2 * base_stat + iv + math.floor(ev / 4)) * self.level) / 100) + 5
                )
        
        return calculated_stats
    
    def _load_move_data(self):
        """Load move data from PokeAPI"""
        print(f"Loading move data for {self.name}...")
        
        for move_name in self.moves:
            try:
                # Try HTTP first, then HTTPS without SSL verification
                urls_to_try = [
                    f"http://pokeapi.co/api/v2/move/{move_name}",
                    f"https://pokeapi.co/api/v2/move/{move_name}"
                ]
                
                move_data = None
                for url in urls_to_try:
                    try:
                        if url.startswith('https'):
                            response = requests.get(url, timeout=10, verify=False, headers={
                                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
                            })
                        else:
                            response = requests.get(url, timeout=10, headers={
                                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
                            })
                        
                        if response.status_code == 200:
                            move_data = response.json()
                            break
                    except Exception:
                        continue
                
                if move_data:
                    self.move_data[move_name] = move_data
                else:
                    # Default move data if API fails
                    self.move_data[move_name] = {
                        'name': move_name,
                        'power': 60,
                        'accuracy': 90,
                        'pp': 20,
                        'type': {'name': 'normal'},
                        'damage_class': {'name': 'physical'},
                        'effect_entries': [{'effect': 'Deals damage with no additional effects.'}]
                    }
                    
            except Exception as e:
                print(f"Warning: Could not load data for move {move_name}: {e}")
                # Use default move data
                self.move_data[move_name] = {
                    'name': move_name,
                    'power': 60,
                    'accuracy': 90,
                    'pp': 20,
                    'type': {'name': 'normal'},
                    'damage_class': {'name': 'physical'},
                    'effect_entries': [{'effect': 'Deals damage with no additional effects.'}],
                    'effect_chance': None,
                    'priority': 0,
                    'crit_rate': 0,
                    'meta': {'category': {'name': 'damage'}}
                }
    
    def _load_item_data(self) -> dict:
        """Load held item data"""
        if not self.held_item:
            return {}
        
        try:
            # Basic item effects - in a full implementation, load from PokeAPI
            item_effects = {
                'leftovers': {'type': 'heal', 'value': 1/16},
                'black-sludge': {'type': 'conditional_heal', 'value': 1/16},
                'choice-band': {'type': 'stat_boost', 'stat': 'attack', 'multiplier': 1.5},
                'choice-specs': {'type': 'stat_boost', 'stat': 'special-attack', 'multiplier': 1.5},
                'choice-scarf': {'type': 'stat_boost', 'stat': 'speed', 'multiplier': 1.5},
                'life-orb': {'type': 'damage_boost', 'multiplier': 1.3, 'recoil': 1/10},
                'focus-sash': {'type': 'survival', 'effect': 'endure'},
                'assault-vest': {'type': 'stat_boost', 'stat': 'special-defense', 'multiplier': 1.5},
                'eviolite': {'type': 'evolution_boost', 'defense': 1.5, 'special-defense': 1.5},
                'flame-orb': {'type': 'self_status', 'status': 'burn'},
                'toxic-orb': {'type': 'self_status', 'status': 'poison'},
                'quick-claw': {'type': 'priority_chance', 'chance': 0.2},
                'muscle-band': {'type': 'type_boost', 'damage_class': 'physical', 'multiplier': 1.1},
                'wise-glasses': {'type': 'type_boost', 'damage_class': 'special', 'multiplier': 1.1},
                'expert-belt': {'type': 'effectiveness_boost', 'multiplier': 1.2},
                'focus-band': {'type': 'survival_chance', 'chance': 0.1}
            }
            
            return item_effects.get(self.held_item.lower(), {})
            
        except Exception as e:
            print(f"Warning: Could not load item data for {self.held_item}: {e}")
            return {}
    
    def get_effective_stat(self, stat_name: str) -> int:
        """Get stat with battle modifications and held item effects applied"""
        base_stat = self.stats.get(stat_name, 0)
        modifier = self.stat_modifiers.get(stat_name, 0)
        
        # Apply stat modification multiplier
        multiplier = 1.0
        if modifier > 0:
            multiplier = (2 + modifier) / 2
        elif modifier < 0:
            multiplier = 2 / (2 + abs(modifier))
        
        effective_stat = int(base_stat * multiplier)
        
        # Apply held item effects
        if self.item_data:
            if self.item_data.get('type') == 'stat_boost' and self.item_data.get('stat') == stat_name:
                effective_stat = int(effective_stat * self.item_data.get('multiplier', 1.0))
            elif self.item_data.get('type') == 'evolution_boost' and stat_name in ['defense', 'special-defense']:
                # Eviolite effect (assuming not fully evolved for simplicity)
                effective_stat = int(effective_stat * self.item_data.get(stat_name.replace('-', '_'), 1.0))
        
        return effective_stat
    
    def get_critical_hit_chance(self, move_name: str) -> float:
        """Calculate critical hit chance based on move and Pokemon state"""
        # Base critical hit ratios by stage
        crit_ratios = {
            0: 1/24,    # Stage 0: 1/24 (4.17%)
            1: 1/8,     # Stage 1: 1/8 (12.5%)
            2: 1/2,     # Stage 2: 1/2 (50%)
            3: 1/1      # Stage 3+: Always crit
        }
        
        move_data = self.move_data.get(move_name, {})
        move_crit_stage = move_data.get('crit_rate', 0)
        
        total_crit_stage = min(3, self.crit_stage + move_crit_stage)
        
        # Some moves always crit (like Frost Breath)
        if move_name.lower() in ['frost-breath', 'storm-throw']:
            return 1.0
        
        return crit_ratios.get(total_crit_stage, 1/24)
    
    def is_fainted(self) -> bool:
        """Check if Pokemon has fainted"""
        return self.current_hp <= 0
    
    def apply_status_damage(self) -> str:
        """Apply damage from status conditions and held items"""
        if not self.status and not self.item_data:
            return ""
        
        messages = []
        
        # Status condition damage
        if self.status == 'poison':
            damage = max(1, self.max_hp // 8)
            self.current_hp = max(0, self.current_hp - damage)
            messages.append(f"{self.name} takes {damage} damage from poison!")
            
        elif self.status == 'burn':
            damage = max(1, self.max_hp // 16)
            self.current_hp = max(0, self.current_hp - damage)
            messages.append(f"{self.name} takes {damage} damage from burn!")
            
        elif self.status == 'badly-poisoned':
            # Badly poisoned increases damage each turn
            damage = max(1, (self.max_hp * self.status_turns) // 16)
            self.current_hp = max(0, self.current_hp - damage)
            messages.append(f"{self.name} takes {damage} damage from severe poison!")
        
        # Held item effects at end of turn
        if self.item_data:
            if self.item_data.get('type') == 'heal':
                heal_amount = int(self.max_hp * self.item_data.get('value', 0))
                if self.current_hp < self.max_hp and self.current_hp > 0:
                    self.current_hp = min(self.max_hp, self.current_hp + heal_amount)
                    messages.append(f"{self.name} restored some HP with {self.held_item.replace('-', ' ').title()}!")
            
            elif self.item_data.get('type') == 'conditional_heal':
                # Black Sludge: heals Poison types, damages others
                heal_amount = int(self.max_hp * self.item_data.get('value', 0))
                if 'poison' in self.types:
                    if self.current_hp < self.max_hp and self.current_hp > 0:
                        self.current_hp = min(self.max_hp, self.current_hp + heal_amount)
                        messages.append(f"{self.name} restored some HP with Black Sludge!")
                else:
                    self.current_hp = max(0, self.current_hp - heal_amount)
                    messages.append(f"{self.name} takes damage from Black Sludge!")
        
        # Decrement status turns for temporary statuses
        if self.status in ['sleep', 'freeze', 'confusion']:
            self.status_turns -= 1
            if self.status_turns <= 0:
                if self.status == 'sleep':
                    messages.append(f"{self.name} woke up!")
                elif self.status == 'freeze':
                    messages.append(f"{self.name} thawed out!")
                elif self.status == 'confusion':
                    messages.append(f"{self.name} snapped out of confusion!")
                self.status = None
                self.status_turns = 0
        
        # Increment badly poisoned counter
        if self.status == 'badly-poisoned':
            self.status_turns += 1
        
        # Reset flinch (only lasts one turn)
        self.flinch = False
        
        return " ".join(messages)

class BattleEngine:
    def __init__(self):
        self.type_effectiveness = self._load_type_effectiveness()
        self.terrain = None  # grassy, misty, electric, psychic
        self.terrain_turns = 0
        self.weather = None  # sun, rain, sandstorm, hail
        self.weather_turns = 0
    
    def set_terrain(self, terrain_type: str, turns: int = 5):
        """Set battlefield terrain"""
        self.terrain = terrain_type
        self.terrain_turns = turns
    
    def set_weather(self, weather_type: str, turns: int = 5):
        """Set battlefield weather"""
        self.weather = weather_type
        self.weather_turns = turns
    
    def _load_type_effectiveness(self) -> dict:
        """Load type effectiveness chart - simplified version"""
        # This is a simplified type chart - in a full implementation you'd load this from PokeAPI
        return {
            'fire': {'grass': 2.0, 'ice': 2.0, 'bug': 2.0, 'steel': 2.0, 'water': 0.5, 'fire': 0.5, 'rock': 0.5, 'dragon': 0.5},
            'water': {'fire': 2.0, 'ground': 2.0, 'rock': 2.0, 'water': 0.5, 'grass': 0.5, 'dragon': 0.5},
            'grass': {'water': 2.0, 'ground': 2.0, 'rock': 2.0, 'fire': 0.5, 'grass': 0.5, 'poison': 0.5, 'flying': 0.5, 'bug': 0.5, 'dragon': 0.5, 'steel': 0.5},
            'electric': {'water': 2.0, 'flying': 2.0, 'electric': 0.5, 'grass': 0.5, 'dragon': 0.5, 'ground': 0.0},
            'psychic': {'fighting': 2.0, 'poison': 2.0, 'psychic': 0.5, 'steel': 0.5, 'dark': 0.0},
            'ice': {'grass': 2.0, 'ground': 2.0, 'flying': 2.0, 'dragon': 2.0, 'fire': 0.5, 'water': 0.5, 'ice': 0.5, 'steel': 0.5},
            'dragon': {'dragon': 2.0, 'steel': 0.5, 'fairy': 0.0},
            'dark': {'psychic': 2.0, 'ghost': 2.0, 'fighting': 0.5, 'dark': 0.5, 'fairy': 0.5},
            'fairy': {'fighting': 2.0, 'dragon': 2.0, 'dark': 2.0, 'fire': 0.5, 'poison': 0.5, 'steel': 0.5},
            'fighting': {'normal': 2.0, 'ice': 2.0, 'rock': 2.0, 'dark': 2.0, 'steel': 2.0, 'poison': 0.5, 'flying': 0.5, 'psychic': 0.5, 'bug': 0.5, 'fairy': 0.5, 'ghost': 0.0},
            'poison': {'grass': 2.0, 'fairy': 2.0, 'poison': 0.5, 'ground': 0.5, 'rock': 0.5, 'ghost': 0.5, 'steel': 0.0},
            'ground': {'fire': 2.0, 'electric': 2.0, 'poison': 2.0, 'rock': 2.0, 'steel': 2.0, 'grass': 0.5, 'bug': 0.5, 'flying': 0.0},
            'flying': {'electric': 0.5, 'ice': 0.5, 'rock': 0.5, 'fighting': 2.0, 'bug': 2.0, 'grass': 2.0, 'steel': 0.5},
            'bug': {'grass': 2.0, 'psychic': 2.0, 'dark': 2.0, 'fire': 0.5, 'fighting': 0.5, 'poison': 0.5, 'flying': 0.5, 'ghost': 0.5, 'steel': 0.5, 'fairy': 0.5},
            'rock': {'fire': 2.0, 'ice': 2.0, 'flying': 2.0, 'bug': 2.0, 'fighting': 0.5, 'ground': 0.5, 'steel': 0.5},
            'ghost': {'psychic': 2.0, 'ghost': 2.0, 'dark': 0.5, 'normal': 0.0},
            'steel': {'ice': 2.0, 'rock': 2.0, 'fairy': 2.0, 'fire': 0.5, 'water': 0.5, 'electric': 0.5, 'steel': 0.5},
            'normal': {'rock': 0.5, 'ghost': 0.0, 'steel': 0.5}
        }
    
    def calculate_type_effectiveness(self, move_type: str, defender_types: List[str]) -> float:
        """Calculate type effectiveness multiplier"""
        effectiveness = 1.0
        
        for defender_type in defender_types:
            type_matchup = self.type_effectiveness.get(move_type, {})
            multiplier = type_matchup.get(defender_type, 1.0)
            effectiveness *= multiplier
        
        return effectiveness
    
    def calculate_damage(self, attacker: Pokemon, defender: Pokemon, move_name: str) -> Tuple[int, str, bool]:
        """Calculate damage dealt by a move with enhanced mechanics"""
        move_data = attacker.move_data.get(move_name)
        if not move_data:
            return 0, "Move data not found!", False
        
        # Get move power - some moves (like status moves) don't have power
        power = move_data.get('power')
        if power is None:
            return 0, "Status move used!", False
        
        move_type = move_data['type']['name']
        damage_class = move_data['damage_class']['name']
        
        # Check for critical hit
        crit_chance = attacker.get_critical_hit_chance(move_name)
        is_critical = random.random() < crit_chance
        
        # Determine attack and defense stats to use
        if damage_class == 'physical':
            attack_stat = attacker.get_effective_stat('attack')
            defense_stat = defender.get_effective_stat('defense')
            
            # Critical hits ignore defense boosts and attack drops
            if is_critical:
                attack_stat = max(attack_stat, attacker.stats['attack'])
                defense_stat = min(defense_stat, defender.stats['defense'])
                
        elif damage_class == 'special':
            attack_stat = attacker.get_effective_stat('special-attack')
            defense_stat = defender.get_effective_stat('special-defense')
            
            # Critical hits ignore defense boosts and attack drops
            if is_critical:
                attack_stat = max(attack_stat, attacker.stats['special-attack'])
                defense_stat = min(defense_stat, defender.stats['special-defense'])
        else:
            # Status moves don't deal damage
            return 0, "Status move used!", False
        
        # Apply burn effect to physical attacks (halves attack)
        if attacker.status == 'burn' and damage_class == 'physical':
            attack_stat = attack_stat // 2
        
        # Base damage calculation
        level_factor = (2 * attacker.level / 5 + 2)
        base_damage = (level_factor * power * attack_stat / defense_stat) / 50 + 2
        
        # Apply STAB (Same Type Attack Bonus)
        stab = 1.5 if move_type in attacker.types else 1.0
        
        # Apply type effectiveness
        type_effectiveness = self.calculate_type_effectiveness(move_type, defender.types)
        
        # Apply critical hit multiplier
        crit_multiplier = 1.5 if is_critical else 1.0
        
        # Apply weather effects
        weather_multiplier = self._get_weather_multiplier(move_type, attacker, defender)
        
        # Apply terrain effects
        terrain_multiplier = self._get_terrain_multiplier(move_type, attacker, defender)
        
        # Apply held item effects
        item_multiplier = self._get_item_damage_multiplier(attacker, defender, move_type, damage_class, type_effectiveness)
        
        # Apply random factor (85-100%)
        random_factor = random.uniform(0.85, 1.0)
        
        # Calculate final damage
        final_damage = int(base_damage * stab * type_effectiveness * crit_multiplier * 
                          weather_multiplier * terrain_multiplier * item_multiplier * random_factor)
        
        # Apply Life Orb recoil
        if attacker.item_data.get('type') == 'damage_boost' and final_damage > 0:
            recoil_damage = int(attacker.max_hp * attacker.item_data.get('recoil', 0))
            attacker.current_hp = max(0, attacker.current_hp - recoil_damage)
        
        # Create effectiveness message
        effectiveness_msg = ""
        if type_effectiveness > 1:
            effectiveness_msg = "It's super effective!"
        elif type_effectiveness < 1 and type_effectiveness > 0:
            effectiveness_msg = "It's not very effective..."
        elif type_effectiveness == 0:
            effectiveness_msg = "It doesn't affect the opponent!"
        
        return max(1, final_damage), effectiveness_msg, is_critical
    
    def use_move(self, attacker: Pokemon, defender: Pokemon, move_name: str) -> List[str]:
        """Execute a move and return battle messages"""
        messages = []
        
        # Check if attacker can use the move
        if attacker.is_fainted():
            return [f"{attacker.name} has fainted and cannot move!"]
        
        # Check flinch
        if attacker.flinch:
            messages.append(f"{attacker.name} flinched and couldn't move!")
            attacker.flinch = False
            return messages
        
        # Check status conditions that prevent moving
        if attacker.status == 'sleep':
            messages.append(f"{attacker.name} is fast asleep!")
            return messages
        
        if attacker.status == 'freeze':
            # 20% chance to thaw out
            if random.random() < 0.2:
                attacker.status = None
                attacker.status_turns = 0
                messages.append(f"{attacker.name} thawed out!")
            else:
                messages.append(f"{attacker.name} is frozen solid!")
                return messages
        
        if attacker.status == 'paralysis' and random.random() < 0.25:
            messages.append(f"{attacker.name} is paralyzed and can't move!")
            return messages
        
        # Check confusion
        if attacker.status == 'confusion':
            messages.append(f"{attacker.name} is confused!")
            if random.random() < 0.33:  # 33% chance to hurt itself
                confusion_damage = int(attacker.max_hp * 0.1)
                attacker.current_hp = max(0, attacker.current_hp - confusion_damage)
                messages.append(f"{attacker.name} hurt itself in confusion for {confusion_damage} damage!")
                return messages
        
        move_data = attacker.move_data.get(move_name)
        if not move_data:
            messages.append(f"{attacker.name} doesn't know {move_name}!")
            return messages
        
        # Check for priority moves and terrain blocking
        move_priority = move_data.get('priority', 0)
        if (self.terrain == 'psychic' and move_priority > 0 and 
            'dark' not in defender.types):  # Psychic Terrain blocks priority moves
            messages.append(f"{attacker.name} used {move_name.replace('-', ' ').title()}!")
            messages.append("The move was blocked by Psychic Terrain!")
            return messages
        
        # Quick Claw check
        quick_claw_activated = False
        if (attacker.item_data.get('type') == 'priority_chance' and 
            random.random() < attacker.item_data.get('chance', 0)):
            quick_claw_activated = True
            messages.append(f"{attacker.name}'s Quick Claw activated!")
        
        # Check accuracy
        accuracy = move_data.get('accuracy')
        if accuracy is None:
            accuracy = 100  # Moves without accuracy always hit (like status moves)
        
        accuracy_modifier = attacker.stat_modifiers.get('accuracy', 0) - defender.stat_modifiers.get('evasion', 0)
        
        # Apply accuracy modification
        final_accuracy = accuracy
        if accuracy_modifier != 0:
            if accuracy_modifier > 0:
                final_accuracy = accuracy * (3 + accuracy_modifier) / 3
            else:
                final_accuracy = accuracy * 3 / (3 + abs(accuracy_modifier))
        
        messages.append(f"{attacker.name} used {move_name.replace('-', ' ').title()}!")
        
        if random.randint(1, 100) > final_accuracy:
            messages.append(f"{attacker.name}'s attack missed!")
            return messages
        
        # Calculate and apply damage
        damage, effectiveness_msg, is_critical = self.calculate_damage(attacker, defender, move_name)
        
        # Focus Sash/Focus Band survival check
        if damage >= defender.current_hp and defender.current_hp == defender.max_hp:
            if defender.item_data.get('type') == 'survival':
                damage = defender.current_hp - 1
                messages.append(f"{defender.name} held on using {defender.held_item.replace('-', ' ').title()}!")
            elif (defender.item_data.get('type') == 'survival_chance' and 
                  random.random() < defender.item_data.get('chance', 0)):
                damage = defender.current_hp - 1
                messages.append(f"{defender.name} held on using Focus Band!")
        
        if damage > 0:
            defender.current_hp = max(0, defender.current_hp - damage)
            messages.append(f"{defender.name} takes {damage} damage!")
            
            if effectiveness_msg:
                messages.append(effectiveness_msg)
            
            if is_critical:
                messages.append("Critical hit!")
        
        # Apply status effects and other move effects
        self._apply_move_effects(attacker, defender, move_name, messages)
        
        return messages
    
    def _apply_move_effects(self, attacker: Pokemon, defender: Pokemon, move_name: str, messages: List[str]):
        """Apply special effects of moves (enhanced implementation)"""
        move_data = attacker.move_data.get(move_name, {})
        move_name_lower = move_name.lower()
        
        # Check for effect chance
        effect_chance_raw = move_data.get('effect_chance', 100)
        effect_chance = (effect_chance_raw or 100) / 100
        
        # Status inflicting moves
        status_moves = {
            'poison': ['poison-powder', 'toxic', 'sludge-bomb', 'poison-jab'],
            'badly-poisoned': ['toxic'],
            'burn': ['will-o-wisp', 'flamethrower', 'fire-blast', 'lava-plume'],
            'paralysis': ['thunder-wave', 'thunderbolt', 'thunder', 'body-slam'],
            'sleep': ['sleep-powder', 'spore', 'hypnosis', 'lovely-kiss'],
            'freeze': ['ice-beam', 'blizzard', 'freeze-dry'],
            'confusion': ['confuse-ray', 'supersonic', 'swagger']
        }
        
        # Apply status effects
        for status, moves in status_moves.items():
            if any(move in move_name_lower for move in moves) and not defender.status:
                if random.random() < effect_chance:
                    defender.status = status
                    if status in ['sleep', 'freeze', 'confusion']:
                        defender.status_turns = random.randint(1, 4)
                    elif status == 'badly-poisoned':
                        defender.status_turns = 1
                    
                    status_names = {
                        'poison': 'poisoned', 'badly-poisoned': 'badly poisoned',
                        'burn': 'burned', 'paralysis': 'paralyzed',
                        'sleep': 'put to sleep', 'freeze': 'frozen',
                        'confusion': 'confused'
                    }
                    messages.append(f"{defender.name} was {status_names[status]}!")
        
        # Stat modification moves
        stat_moves = {
            'attack': {'swords-dance': 2, 'howl': 1, 'meditate': 1},
            'defense': {'harden': 1, 'withdraw': 1, 'iron-defense': 2},
            'special-attack': {'nasty-plot': 2, 'calm-mind': 1},
            'special-defense': {'amnesia': 2, 'calm-mind': 1},
            'speed': {'agility': 2, 'quick-attack': 1, 'flame-charge': 1},
            'accuracy': {'hone-claws': 1},
            'evasion': {'double-team': 1, 'minimize': 2}
        }
        
        # Apply stat modifications
        for stat, moves_dict in stat_moves.items():
            for move, boost in moves_dict.items():
                if move in move_name_lower:
                    target = attacker if 'self' in move_data.get('effect_entries', [{}])[0].get('effect', '') else attacker
                    
                    old_modifier = target.stat_modifiers[stat]
                    target.stat_modifiers[stat] = min(6, max(-6, target.stat_modifiers[stat] + boost))
                    
                    if target.stat_modifiers[stat] != old_modifier:
                        if boost > 0:
                            level_text = {1: "rose", 2: "rose sharply", 3: "rose drastically"}
                            messages.append(f"{target.name}'s {stat.replace('-', ' ')} {level_text.get(boost, 'rose')}!")
                        else:
                            level_text = {-1: "fell", -2: "fell sharply", -3: "fell drastically"}
                            messages.append(f"{target.name}'s {stat.replace('-', ' ')} {level_text.get(boost, 'fell')}!")
        
        # Weather moves
        weather_moves = {
            'sunny-day': 'sun', 'rain-dance': 'rain',
            'sandstorm': 'sandstorm', 'hail': 'hail'
        }
        
        for move, weather in weather_moves.items():
            if move in move_name_lower:
                self.set_weather(weather, 5)
                weather_text = {
                    'sun': 'The sunlight turned harsh!',
                    'rain': 'It started to rain!',
                    'sandstorm': 'A sandstorm kicked up!',
                    'hail': 'It started to hail!'
                }
                messages.append(weather_text[weather])
        
        # Terrain moves
        terrain_moves = {
            'grassy-terrain': 'grassy', 'misty-terrain': 'misty',
            'electric-terrain': 'electric', 'psychic-terrain': 'psychic'
        }
        
        for move, terrain in terrain_moves.items():
            if move in move_name_lower:
                self.set_terrain(terrain, 5)
                terrain_text = {
                    'grassy': 'Grass grew to cover the battlefield!',
                    'misty': 'Mist swirled around the battlefield!',
                    'electric': 'An electric current ran across the battlefield!',
                    'psychic': 'The battlefield got weird!'
                }
                messages.append(terrain_text[terrain])
        
        # Flinch moves
        flinch_moves = ['fake-out', 'air-slash', 'iron-head', 'rock-slide']
        if any(move in move_name_lower for move in flinch_moves):
            if random.random() < effect_chance:
                defender.flinch = True
                messages.append(f"{defender.name} flinched!")
        
        # Critical hit ratio boosting moves
        crit_moves = ['focus-energy', 'laser-focus']
        if any(move in move_name_lower for move in crit_moves):
            attacker.crit_stage = min(3, attacker.crit_stage + 1)
            messages.append(f"{attacker.name} is getting pumped!")
        
        # Self-status moves (like Rest, Flame Orb activation, etc.)
        if 'rest' in move_name_lower:
            attacker.current_hp = attacker.max_hp
            attacker.status = 'sleep'
            attacker.status_turns = 2
            messages.append(f"{attacker.name} went to sleep and restored its HP!")
        
        # Multi-hit moves (simplified)
        multi_hit_moves = ['double-slap', 'fury-attack', 'pin-missile', 'rock-blast']
        if any(move in move_name_lower for move in multi_hit_moves):
            hits = random.choice([2, 2, 3, 3, 4, 5])  # Weighted toward 2-3 hits
            if hits > 1:
                messages.append(f"Hit {hits} times!")
    
    def battle_turn(self, pokemon1: Pokemon, pokemon2: Pokemon, move1: str, move2: str) -> List[str]:
        """Execute one turn of battle"""
        messages = []
        
        # Determine turn order based on speed and priority
        move1_data = pokemon1.move_data.get(move1, {})
        move2_data = pokemon2.move_data.get(move2, {})
        
        priority1 = move1_data.get('priority', 0)
        priority2 = move2_data.get('priority', 0)
        
        speed1 = pokemon1.get_effective_stat('speed')
        speed2 = pokemon2.get_effective_stat('speed')
        
        # Paralysis halves speed
        if pokemon1.status == 'paralysis':
            speed1 = speed1 // 4
        if pokemon2.status == 'paralysis':
            speed2 = speed2 // 4
        
        # Quick Claw can change priority
        quick_claw1 = (pokemon1.item_data.get('type') == 'priority_chance' and 
                      random.random() < pokemon1.item_data.get('chance', 0))
        quick_claw2 = (pokemon2.item_data.get('type') == 'priority_chance' and 
                      random.random() < pokemon2.item_data.get('chance', 0))
        
        # Determine order
        if priority1 > priority2 or (priority1 == priority2 and (speed1 > speed2 or 
           (speed1 == speed2 and random.choice([True, False])))):
            first, second = pokemon1, pokemon2
            first_move, second_move = move1, move2
        else:
            first, second = pokemon2, pokemon1
            first_move, second_move = move2, move1
        
        # First Pokemon attacks
        messages.extend(self.use_move(first, second, first_move))
        
        # Second Pokemon attacks (if not fainted)
        if not second.is_fainted():
            messages.extend(self.use_move(second, first, second_move))
        
        # Apply status damage
        for pokemon in [pokemon1, pokemon2]:
            if not pokemon.is_fainted():
                status_msg = pokemon.apply_status_damage()
                if status_msg:
                    messages.append(status_msg)
        
        # Apply weather effects
        weather_messages = self._apply_weather_effects(pokemon1, pokemon2)
        messages.extend(weather_messages)
        
        # Apply terrain effects  
        terrain_messages = self._apply_terrain_effects(pokemon1, pokemon2)
        messages.extend(terrain_messages)
        
        # Decrement weather and terrain turns
        if self.weather_turns > 0:
            self.weather_turns -= 1
            if self.weather_turns == 0:
                messages.append(f"The {self.weather} stopped!")
                self.weather = None
        
        if self.terrain_turns > 0:
            self.terrain_turns -= 1
            if self.terrain_turns == 0:
                messages.append(f"The {self.terrain} terrain faded!")
                self.terrain = None
        
        return messages
    
    def _apply_weather_effects(self, pokemon1: Pokemon, pokemon2: Pokemon) -> List[str]:
        """Apply weather effects at end of turn"""
        messages = []
        
        if not self.weather:
            return messages
        
        for pokemon in [pokemon1, pokemon2]:
            if pokemon.is_fainted():
                continue
            
            # Sandstorm damage
            if self.weather == 'sandstorm':
                if not any(ptype in ['rock', 'ground', 'steel'] for ptype in pokemon.types):
                    damage = max(1, pokemon.max_hp // 16)
                    pokemon.current_hp = max(0, pokemon.current_hp - damage)
                    messages.append(f"{pokemon.name} is buffeted by the sandstorm!")
            
            # Hail damage
            elif self.weather == 'hail':
                if 'ice' not in pokemon.types:
                    damage = max(1, pokemon.max_hp // 16)
                    pokemon.current_hp = max(0, pokemon.current_hp - damage)
                    messages.append(f"{pokemon.name} is buffeted by the hail!")
        
        return messages
    
    def _apply_terrain_effects(self, pokemon1: Pokemon, pokemon2: Pokemon) -> List[str]:
        """Apply terrain effects at end of turn"""
        messages = []
        
        if not self.terrain:
            return messages
        
        # Grassy Terrain healing
        if self.terrain == 'grassy':
            for pokemon in [pokemon1, pokemon2]:
                if pokemon.is_fainted():
                    continue
                
                # Heal grounded Pokemon
                heal_amount = max(1, pokemon.max_hp // 16)
                if pokemon.current_hp < pokemon.max_hp:
                    pokemon.current_hp = min(pokemon.max_hp, pokemon.current_hp + heal_amount)
                    messages.append(f"{pokemon.name} restored some HP due to Grassy Terrain!")
        
        return messages

    def _get_weather_multiplier(self, move_type: str, attacker: Pokemon, defender: Pokemon) -> float:
        """Calculate weather effects on damage"""
        if not self.weather:
            return 1.0
        
        # Sun effects
        if self.weather == 'sun':
            if move_type == 'fire':
                return 1.5
            elif move_type == 'water':
                return 0.5
        
        # Rain effects
        elif self.weather == 'rain':
            if move_type == 'water':
                return 1.5
            elif move_type == 'fire':
                return 0.5
        
        return 1.0
    
    def _get_terrain_multiplier(self, move_type: str, attacker: Pokemon, defender: Pokemon) -> float:
        """Calculate terrain effects on damage"""
        if not self.terrain:
            return 1.0
        
        # Grassy Terrain: Boosts Grass moves, reduces Earthquake/Magnitude/Bulldoze damage
        if self.terrain == 'grassy':
            if move_type == 'grass':
                return 1.3
            # Note: Would need move name to check for ground moves that hit grounded Pokemon
        
        # Electric Terrain: Boosts Electric moves
        elif self.terrain == 'electric':
            if move_type == 'electric':
                return 1.3
        
        # Psychic Terrain: Boosts Psychic moves, blocks priority moves
        elif self.terrain == 'psychic':
            if move_type == 'psychic':
                return 1.3
        
        # Misty Terrain: Reduces Dragon move damage
        elif self.terrain == 'misty':
            if move_type == 'dragon':
                return 0.5
        
        return 1.0
    
    def _get_item_damage_multiplier(self, attacker: Pokemon, defender: Pokemon, move_type: str, damage_class: str, type_effectiveness: float) -> float:
        """Calculate held item effects on damage"""
        multiplier = 1.0
        
        # Attacker's item effects
        if attacker.item_data:
            item_type = attacker.item_data.get('type')
            
            if item_type == 'damage_boost':
                multiplier *= attacker.item_data.get('multiplier', 1.0)
            
            elif item_type == 'type_boost':
                if attacker.item_data.get('damage_class') == damage_class:
                    multiplier *= attacker.item_data.get('multiplier', 1.0)
            
            elif item_type == 'effectiveness_boost' and type_effectiveness > 1:
                multiplier *= attacker.item_data.get('multiplier', 1.0)
        
        # Defender's item effects (like Assault Vest already applied to stats)
        
        return multiplier

class BattleSimulator:
    def __init__(self):
        self.battle_engine = BattleEngine()
    
    def simulate_battle(self, pokemon1: Pokemon, pokemon2: Pokemon, auto_battle: bool = False) -> str:
        """Simulate a complete battle between two Pokemon"""
        print(f"\n{'='*60}")
        print(f"POKEMON BATTLE: {pokemon1.name} vs {pokemon2.name}")
        print(f"{'='*60}")
        
        # Display Pokemon info
        self._display_battle_info(pokemon1, pokemon2)
        
        turn_count = 0
        max_turns = 100  # Prevent infinite battles
        
        while not pokemon1.is_fainted() and not pokemon2.is_fainted() and turn_count < max_turns:
            turn_count += 1
            print(f"\n--- Turn {turn_count} ---")
            
            # Display current battle conditions
            conditions = []
            if self.battle_engine.weather:
                conditions.append(f"Weather: {self.battle_engine.weather.title()} ({self.battle_engine.weather_turns} turns left)")
            if self.battle_engine.terrain:
                conditions.append(f"Terrain: {self.battle_engine.terrain.title()} ({self.battle_engine.terrain_turns} turns left)")
            if conditions:
                print(f"Conditions: {', '.join(conditions)}")
            
            # Get moves for this turn
            if auto_battle:
                move1 = random.choice(pokemon1.moves)
                move2 = random.choice(pokemon2.moves)
                print(f"{pokemon1.name} will use {move1.replace('-', ' ').title()}")
                print(f"{pokemon2.name} will use {move2.replace('-', ' ').title()}")
            else:
                move1 = self._get_player_move(pokemon1, 1)
                move2 = self._get_player_move(pokemon2, 2)
            
            # Execute turn
            turn_messages = self.battle_engine.battle_turn(pokemon1, pokemon2, move1, move2)
            
            # Display turn results
            for message in turn_messages:
                print(message)
                if not auto_battle:
                    time.sleep(0.5)  # Dramatic pause for manual battles
            
            # Display current HP and status
            def format_status(pokemon):
                status_text = ""
                if pokemon.status:
                    status_text = f" ({pokemon.status.upper()})"
                if pokemon.held_item:
                    status_text += f" [Item: {pokemon.held_item.replace('-', ' ').title()}]"
                return status_text
            
            print(f"\n{pokemon1.name}: {pokemon1.current_hp}/{pokemon1.max_hp} HP{format_status(pokemon1)}")
            print(f"{pokemon2.name}: {pokemon2.current_hp}/{pokemon2.max_hp} HP{format_status(pokemon2)}")
            
            if auto_battle and turn_count % 3 == 0:
                time.sleep(1)  # Pause every few turns in auto battle
        
        # Determine winner
        if pokemon1.is_fainted() and pokemon2.is_fainted():
            winner = "It's a tie!"
        elif pokemon1.is_fainted():
            winner = f"{pokemon2.name} wins!"
        elif pokemon2.is_fainted():
            winner = f"{pokemon1.name} wins!"
        else:
            winner = "Battle reached maximum turns - it's a draw!"
        
        print(f"\n{'='*60}")
        print(f"BATTLE RESULT: {winner}")
        print(f"Battle lasted {turn_count} turns")
        print(f"{'='*60}")
        
        return winner
    
    def _display_battle_info(self, pokemon1: Pokemon, pokemon2: Pokemon):
        """Display initial battle information"""
        for i, pokemon in enumerate([pokemon1, pokemon2], 1):
            print(f"\nPokemon {i}: {pokemon.name}")
            print(f"  Level: {pokemon.level}")
            print(f"  Types: {', '.join(pokemon.types).title()}")
            if pokemon.held_item:
                print(f"  Held Item: {pokemon.held_item.replace('-', ' ').title()}")
            print(f"  HP: {pokemon.max_hp}")
            print(f"  Attack: {pokemon.stats['attack']}")
            print(f"  Defense: {pokemon.stats['defense']}")
            print(f"  Sp. Atk: {pokemon.stats['special-attack']}")
            print(f"  Sp. Def: {pokemon.stats['special-defense']}")
            print(f"  Speed: {pokemon.stats['speed']}")
            print(f"  Moves: {', '.join([move.replace('-', ' ').title() for move in pokemon.moves])}")
    
    def _get_player_move(self, pokemon: Pokemon, player_num: int) -> str:
        """Get move choice from player"""
        print(f"\nPlayer {player_num} ({pokemon.name}), choose your move:")
        for i, move in enumerate(pokemon.moves, 1):
            move_data = pokemon.move_data.get(move, {})
            power = move_data.get('power', 'N/A')
            move_type = move_data.get('type', {}).get('name', 'unknown')
            print(f"  {i}. {move.replace('-', ' ').title()} (Type: {move_type.title()}, Power: {power})")
        
        while True:
            try:
                choice = input("Enter move number (1-4): ").strip()
                move_index = int(choice) - 1
                if 0 <= move_index < len(pokemon.moves):
                    return pokemon.moves[move_index]
                else:
                    print("Invalid choice. Please enter 1-4.")
            except ValueError:
                print("Please enter a valid number.")
